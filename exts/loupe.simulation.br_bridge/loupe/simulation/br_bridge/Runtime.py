"""
  File: **Runtime.py**
  Copyright (c) 2024 Loupe
  https://loupe.team

  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.

  The Kit side of one PLC. The polling itself is plc_bridge.PlcRuntime driving a
  br_bridge.BrDriver (OMJSON over a websocket), both plain Python. This class
  only connects that runtime to Kit: options from the PLC prim in, carb message
  bus events out, bus read and write requests in.
"""

import logging

import omni.kit.app

from br_bridge import BrDriver
from plc_bridge import PlcRuntime

from ..common.RuntimeBase import get_stream_name

from .global_variables import (
    ATTR_BR_BRIDGE_HOST,
    ATTR_BR_BRIDGE_PORT,
    ATTR_BR_BRIDGE_ENABLE,
    ATTR_BR_BRIDGE_READ_VARS,
    ATTR_BR_BRIDGE_REFRESH,
)  # noqa: E501

from .BrBridge import (
    EVENT_TYPE_DATA_READ,
    EVENT_TYPE_DATA_READ_REQ,
    EVENT_TYPE_DATA_WRITE_REQ,
    EVENT_TYPE_DATA_INIT,
    EVENT_TYPE_CONNECTION,
    EVENT_TYPE_ENABLE,
    EVENT_TYPE_STATUS,
)

logger = logging.getLogger(__name__)


def _option(options: dict, key: str, default):
    """
    Read an option, falling back to the default when the key is missing or None.
    Do not use `options.get(key) or default`: that turns a legitimate False or 0
    into the default, so the option can never be switched off.
    """
    value = options.get(key)
    return default if value is None else value


class Runtime:
    # region - Class lifecycle
    def __init__(self, name="PLC1", options=None):
        options = options or {}
        self._name = name

        self._driver = BrDriver(
            _option(options, ATTR_BR_BRIDGE_HOST, "127.0.0.1"),
            int(_option(options, ATTR_BR_BRIDGE_PORT, 8000)),
        )
        self._plc = PlcRuntime(
            self._driver,
            name=name,
            refresh_ms=_option(options, ATTR_BR_BRIDGE_REFRESH, 20),
            enabled=_option(options, ATTR_BR_BRIDGE_ENABLE, False),
        )
        self._plc.set_read_variables((options.get(ATTR_BR_BRIDGE_READ_VARS) or "").split(","))

        # Runtime events -> message bus. These run on the runtime's worker thread;
        # subscribers that touch the stage or the UI marshal to the main thread
        # themselves (see RuntimeUsd).
        self._plc.on_data(lambda data: self._push_event(EVENT_TYPE_DATA_READ, data=data))
        self._plc.on_status(lambda text: self._push_event(EVENT_TYPE_STATUS, status=text))
        self._plc.on_connection(lambda state: self._push_event(EVENT_TYPE_CONNECTION, status=state))
        self._plc.on_enabled(
            lambda enabled: self._push_event(EVENT_TYPE_ENABLE, status={"enabled": enabled}))

        # Message bus requests -> runtime
        self._event_stream = omni.kit.app.get_app().get_message_bus_event_stream()
        self._read_req = self._event_stream.create_subscription_to_push_by_type(
            self._get_stream_name(EVENT_TYPE_DATA_READ_REQ), self._on_read_req_event
        )
        self._write_req = self._event_stream.create_subscription_to_push_by_type(
            self._get_stream_name(EVENT_TYPE_DATA_WRITE_REQ), self._on_write_req_event
        )
        self._push_event(EVENT_TYPE_DATA_INIT, data={})

        self._plc.start()

    def __del__(self):
        self.cleanup()

    def cleanup(self):
        """Stop polling, drop the websocket loop and leave the message bus. Safe to call more than once."""
        plc = getattr(self, "_plc", None)  # __init__ may have failed before it existed
        if plc is not None:
            plc.stop()
        driver = getattr(self, "_driver", None)
        if driver is not None:
            # The driver owns an event-loop thread for the websocket; stop() above
            # only disconnects. close() ends the thread so a runtime that is torn
            # down on every stage open/close does not leave one behind.
            driver.close()
        for attr in ("_read_req", "_write_req"):
            subscription = getattr(self, attr, None)
            if subscription is not None:
                subscription.unsubscribe()
                setattr(self, attr, None)

    # endregion
    # region - Properties
    name = property(lambda self: self._name)
    plc = property(lambda self: self._plc, doc="The plc_bridge.PlcRuntime doing the polling.")
    driver = property(lambda self: self._driver, doc="The br_bridge.BrDriver.")
    is_connected = property(lambda self: self._plc.is_connected)
    read_variables = property(lambda self: self._plc.read_variables)

    host = property(
        lambda self: self._driver.host,
        lambda self, value: self._set_endpoint(host=value),
    )

    port = property(
        lambda self: self._driver.port,
        lambda self, value: self._set_endpoint(port=int(value)),
    )

    def _set_endpoint(self, host=None, port=None):
        changed = False
        if host is not None and host != self._driver.host:
            self._driver.host = host
            changed = True
        if port is not None and port != self._driver.port:
            self._driver.port = port
            changed = True
        if changed:
            self._plc.reconnect()

    enable_communication = property(
        lambda self: self._plc.enabled,
        lambda self, value: setattr(self._plc, "enabled", value),
    )

    refresh_period_ms = property(
        lambda self: self._plc.refresh_ms,
        lambda self, value: setattr(self._plc, "refresh_ms", value),
    )
    # Two names for one value; the prim attribute is called RefreshRate.
    refresh_rate = refresh_period_ms

    write_sleep_time = property(
        lambda self: self._plc.write_sleep,
        lambda self, value: setattr(self._plc, "write_sleep", value),
    )

    @property
    def options(self):
        return {
            ATTR_BR_BRIDGE_HOST: self.host,
            ATTR_BR_BRIDGE_PORT: self.port,
            ATTR_BR_BRIDGE_ENABLE: self.enable_communication,
            ATTR_BR_BRIDGE_REFRESH: self.refresh_rate,
            ATTR_BR_BRIDGE_READ_VARS: ",".join(self._plc.read_variables),
        }

    @options.setter
    def options(self, value):
        # _option: a prim attribute with no value arrives as None and must not
        # replace a good setting (see __init__).
        self._set_endpoint(
            host=_option(value, ATTR_BR_BRIDGE_HOST, self.host),
            port=int(_option(value, ATTR_BR_BRIDGE_PORT, self.port)),
        )
        # Always assigned: the assignment pushes the ENABLE event that tells
        # listeners the options were (re)applied.
        self.enable_communication = _option(
            value, ATTR_BR_BRIDGE_ENABLE, self.enable_communication
        )
        self.refresh_rate = _option(value, ATTR_BR_BRIDGE_REFRESH, self.refresh_rate)
        # The variables option replaces the cyclic read list, so a variable removed
        # from the prim stops being read. A missing key leaves the list alone.
        if ATTR_BR_BRIDGE_READ_VARS in value:
            variables = value[ATTR_BR_BRIDGE_READ_VARS] or ""
            self.set_read_variables(variables.split(","))

    # endregion
    # region - Message bus
    def _get_stream_name(self, msg_type):
        return get_stream_name(msg_type, self._name)

    def _push_event(self, event_type, data=None, status=None):
        message = {"meta": {"name": self._name}}
        if data:
            message["data"] = data
        if status:
            message["status"] = status
        try:
            self._event_stream.push(
                event_type=self._get_stream_name(event_type), payload=message)
        except Exception as e:
            logger.error(f"Error pushing event: {e}")

    def _on_read_req_event(self, event):
        self._plc.add_read_variables(event.payload["variables"])

    def _on_write_req_event(self, event):
        for variable in event.payload["variables"]:
            self.queue_write(variable["name"], variable["value"])

    # endregion
    # region - External API
    def set_read_variables(self, variables):
        """
        Replace the cyclic read list. Blank entries are dropped and whitespace is
        stripped (see PlcRuntime.set_read_variables).
        """
        self._plc.set_read_variables(variables)

    def resolve_symbol(self, name: str) -> str:
        """
        Map a symbol spelled with "." only back to the OMJSON name the PLC knows.

        The shared USD mirror (loupe.simulation.common.UsdManager) rebuilds symbol
        names from the nested data with "." between every part, so a write:value
        edit on /PLC/PLC1/TestProg/lreal asks to write "TestProg.lreal", which
        OMJSON does not know: a task-local variable is "TestProg:lreal". The cyclic
        read list holds the real names, so a request that matches one of them
        with the separators normalised is sent under that name. Anything else is
        sent as given. Goes away with Phase 3.7 of the implementation plan, when
        the mirror keeps the flat symbol instead of re-deriving it.
        """
        if ":" in name:
            return name
        for symbol in self._plc.read_variables:
            if symbol.replace(":", ".") == name:
                return symbol
        return name

    def queue_write(self, name, value):
        return self._plc.queue_write(self.resolve_symbol(name), value)

    # endregion
