"""
A mock OMJSON server for the tests: the protocol of jsonWebSocketServer, with
a dict of variables instead of a PLC. Runs on its own thread and loop so a
synchronous test can start, drive and stop it.
"""

import asyncio
import json
import threading

import websockets

UNDEFINED = "undefined"


class MockOmjson:
    def __init__(self, variables: dict, silent: bool = False):
        self.variables = dict(variables)
        self.silent = silent          # accept connections, never reply
        self.requests = []
        self.port = None
        self._loop = None
        self._server = None
        self._thread = None

    async def _handle(self, websocket):
        async for message in websocket:
            if message == "":
                continue  # the driver's pre-close frame
            request = json.loads(message)
            self.requests.append(request)
            if self.silent:
                continue
            if request["type"] == "read":
                data = []
                for symbol in request["data"]:
                    if symbol in self.variables:
                        data.append({symbol: self.variables[symbol]})
                    elif symbol.startswith("undef"):
                        data.append({symbol: UNDEFINED})
                    # unknown symbols are simply left out, like a real server
                await websocket.send(json.dumps({"type": "readresponse", "data": data}))
            elif request["type"] == "write":
                data = {}
                for symbol, value in request["data"].items():
                    if symbol in self.variables:
                        self.variables[symbol] = value
                        data[symbol] = value
                    elif symbol.startswith("undef"):
                        data[symbol] = UNDEFINED
                await websocket.send(json.dumps({"type": "writeresponse", "data": data}))

    def start(self):
        started = threading.Event()

        async def main():
            self._server = await websockets.serve(self._handle, "127.0.0.1", 0)
            self.port = self._server.sockets[0].getsockname()[1]
            started.set()
            await asyncio.Future()

        def run():
            self._loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self._loop)
            try:
                self._loop.run_until_complete(main())
            except asyncio.CancelledError:
                pass

        self._thread = threading.Thread(target=run, name="mock-omjson", daemon=True)
        self._thread.start()
        assert started.wait(5), "mock server did not start"
        return self

    def stop(self):
        loop = self._loop
        if loop is None:
            return

        async def shutdown():
            self._server.close()
            await self._server.wait_closed()
            for task in asyncio.all_tasks(loop):
                task.cancel()

        asyncio.run_coroutine_threadsafe(shutdown(), loop)
        self._thread.join(5)
