"""
  File: **driver.py**
  Copyright (c) 2024 Loupe
  https://loupe.team

  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.

  The B&R driver: OMJSON over a websocket, implementing the plc_bridge driver
  contract. Plain Python: nothing here may import from Omniverse or Kit.

  Protocol (https://loupeteam.github.io/LoupeDocs/libraries/omjson/jsonwebsocketserver.html):
    -> {"type": "read",  "data": ["globalVar", "myTask:localVar"]}
    <- {"type": "readresponse",  "data": [{"globalVar": 25}, {"myTask:localVar": {...}}]}
    -> {"type": "write", "data": {"globalVar": 52}}
    <- {"type": "writeresponse", "data": {"globalVar": 52}}
  A symbol the PLC does not know, or cannot represent, comes back as "undefined"
  in a readresponse. A writeresponse does not say so: OMJSON 2.0.0 echoes the
  request, unknown symbols included, so a write to a symbol that does not exist
  looks written. The driver flags such a write only when the symbol's most
  recent read failed. A write the server cannot parse gets no reply at all, so
  it times out and the link is reported lost.
"""

import asyncio
import json
import logging
import threading
from typing import Any, Mapping, Optional, Sequence

import websockets

from plc_bridge import PlcDriver, ReadResult

logger = logging.getLogger(__name__)

# What OMJSON returns for a symbol it does not know or cannot represent.
UNDEFINED = "undefined"
# What a symbol missing from the response is reported as.
NOT_IN_RESPONSE = "not in response"


class BrDriver(PlcDriver):
    """
    One OMJSON websocket connection to one B&R PLC.

    The transport is asyncio; the driver owns an event loop on a daemon thread
    of its own and exposes the synchronous contract on top of it. One request
    is in flight at a time (OMJSON has no correlation ids, so a reply can only
    be matched to the request that preceded it), which the single-worker
    runtime guarantees and a lock enforces for other callers.

    Args:
        host: PLC address.
        port: OMJSON server port (jsonWebSocketServer default 8000).
        timeout: seconds to wait for a connect, a reply, or a close.
    """

    symbol_separators = ":."

    def __init__(self, host: str, port: int = 8000, timeout: float = 3.0):
        self.host = host
        self.port = port
        self.timeout = timeout
        self._ws = None
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._loop_thread: Optional[threading.Thread] = None
        self._request_lock = threading.Lock()
        self._transport_lost = False
        # Why each symbol's most recent read failed; a later good read clears it.
        self._read_errors = {}

    # region - Event loop

    def _ensure_loop(self) -> asyncio.AbstractEventLoop:
        if self._loop is not None and self._loop.is_running():
            return self._loop
        loop = asyncio.new_event_loop()
        started = threading.Event()

        def run():
            asyncio.set_event_loop(loop)
            loop.call_soon(started.set)
            loop.run_forever()

        thread = threading.Thread(target=run, name=f"br-{self.host}:{self.port}", daemon=True)
        thread.start()
        started.wait(self.timeout)
        self._loop, self._loop_thread = loop, thread
        return loop

    def _call(self, coro, timeout: Optional[float] = None):
        """Run a coroutine on the driver's loop and wait for it, bounded."""
        future = asyncio.run_coroutine_threadsafe(coro, self._ensure_loop())
        try:
            return future.result(self.timeout if timeout is None else timeout)
        except TimeoutError:
            future.cancel()
            raise TimeoutError(f"OMJSON at {self.host}:{self.port}: no reply within {self.timeout}s")
        except asyncio.TimeoutError:
            future.cancel()
            raise TimeoutError(f"OMJSON at {self.host}:{self.port}: no reply within {self.timeout}s")

    def close(self):
        """Disconnect and stop the loop thread. The driver cannot be used afterwards."""
        self.disconnect()
        loop = self._loop
        if loop is not None and loop.is_running():
            loop.call_soon_threadsafe(loop.stop)
            if self._loop_thread is not None:
                self._loop_thread.join(self.timeout)
        self._loop = None

    # endregion
    # region - Contract

    @property
    def uri(self) -> str:
        return f"ws://{self.host}:{self.port}"

    def connect(self) -> None:
        self.disconnect()
        self._transport_lost = False
        self._ws = self._call(self._open(), timeout=self.timeout + 1)

    async def _open(self):
        # websockets.connect() returns an awaitable helper, not a coroutine, so
        # run_coroutine_threadsafe needs this wrapper.
        return await websockets.connect(
            self.uri,
            open_timeout=self.timeout,
            ping_interval=None,   # OMJSON does not answer pings
            close_timeout=1,
        )

    def disconnect(self) -> None:
        ws, self._ws = self._ws, None
        if ws is None:
            return
        try:
            self._call(self._close(ws), timeout=self.timeout + 1)
        except Exception:  # noqa: a close that fails is still a close
            pass

    @staticmethod
    async def _close(ws):
        # OMJSON does not handle the close opcode well; an empty frame first
        # lets the PLC drop the client before the socket goes away.
        try:
            await ws.send("")
        except Exception:
            pass
        await ws.close()

    def is_connected(self) -> bool:
        ws = self._ws
        if ws is None or self._transport_lost:
            return False
        try:
            return ws.state is websockets.protocol.State.OPEN
        except AttributeError:  # websockets < 13
            return bool(getattr(ws, "open", False))

    def read(self, symbols: Sequence[str]) -> ReadResult:
        symbols = list(symbols)
        response = self._rpc({"type": "read", "data": symbols})
        if response.get("type") != "readresponse":
            raise ValueError(f"OMJSON: expected a readresponse, got {response.get('type')!r}")
        result = ReadResult()
        returned = {}
        for entry in response.get("data") or []:
            if isinstance(entry, dict):
                returned.update(entry)
        for symbol in symbols:
            if symbol not in returned:
                result.errors[symbol] = NOT_IN_RESPONSE
            elif returned[symbol] == UNDEFINED:
                result.errors[symbol] = UNDEFINED
            else:
                result.values[symbol] = returned[symbol]
                self._read_errors.pop(symbol, None)
        self._read_errors.update(result.errors)
        return result

    def write(self, values: Mapping[str, Any]) -> Mapping[str, str]:
        values = dict(values)
        response = self._rpc({"type": "write", "data": values})
        if response.get("type") != "writeresponse":
            raise ValueError(f"OMJSON: expected a writeresponse, got {response.get('type')!r}")
        returned = response.get("data") or {}
        errors = {}
        for symbol in values:
            if symbol not in returned:
                errors[symbol] = NOT_IN_RESPONSE
            elif returned[symbol] == UNDEFINED:
                errors[symbol] = UNDEFINED
            elif symbol in self._read_errors:
                # OMJSON echoes unknown symbols as written; the last read knows better.
                errors[symbol] = f"{self._read_errors[symbol]} on the last read"
        return errors

    # endregion
    # region - Transport

    def _rpc(self, request: dict) -> dict:
        ws = self._ws
        if ws is None:
            raise ConnectionError(f"OMJSON at {self.host}:{self.port}: not connected")
        with self._request_lock:
            try:
                reply = self._call(self._exchange(ws, json.dumps(request)))
            except TimeoutError:
                self._transport_lost = True
                raise
            except websockets.exceptions.ConnectionClosed as e:
                self._transport_lost = True
                raise ConnectionError(f"OMJSON at {self.host}:{self.port}: connection closed ({e})") from e
            except OSError as e:
                self._transport_lost = True
                raise ConnectionError(f"OMJSON at {self.host}:{self.port}: {e}") from e
        try:
            return json.loads(reply)
        except (TypeError, ValueError) as e:
            raise ValueError(f"OMJSON: reply is not JSON: {reply!r}") from e

    @staticmethod
    async def _exchange(ws, payload: str) -> str:
        await ws.send(payload)
        return await ws.recv()

    # endregion
