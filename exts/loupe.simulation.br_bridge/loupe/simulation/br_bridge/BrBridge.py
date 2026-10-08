"""
Copyright (c) 2024 Loupe, https://loupe.team. Part of Omniverse_BnR_Bridge_Extension, MIT License.

DEPRECATED compatibility module. The script API moved to the framework
extension `loupe.simulation.bridge`; this module re-exports it on the legacy
`br_bridge` namespace (events `loupe.simulation.br_bridge.<KIND>.<plc>`) so
existing scripts run unchanged. 0.3 warns, 0.4 turns those bus names off by
default (framework setting `legacyBusNames`), 0.5 removes this module.
"""

import logging
import warnings

import carb.settings
import omni.kit.app
from loupe.simulation.bridge import Manager as _Manager
from loupe.simulation.bridge import Manager_Events as _ManagerEvents
from loupe.simulation.bridge import get_system, registry
from loupe.simulation.bridge.bus import get_stream_name  # noqa: F401  re-exported

from .extension import DRIVER_NAME, LEGACY_NAMESPACE

logger = logging.getLogger(__name__)

_MESSAGE = ("loupe.simulation.br_bridge.BrBridge is DEPRECATED and will be removed in 0.5: import Manager, "
            "get_system, get_plc and on_sample_main from loupe.simulation.bridge. Its bus names "
            "(loupe.simulation.br_bridge.*) are off by default from 0.4.")
warnings.warn(_MESSAGE, DeprecationWarning, stacklevel=2)
logger.warning(_MESSAGE)

br_bridge_name = LEGACY_NAMESPACE
Manager_Events = _ManagerEvents(LEGACY_NAMESPACE)
EVENT_TYPE_DATA_INIT = Manager_Events.EVENT_TYPE_DATA_INIT
EVENT_TYPE_DATA_READ = Manager_Events.EVENT_TYPE_DATA_READ
EVENT_TYPE_DATA_READ_REQ = Manager_Events.EVENT_TYPE_DATA_READ_REQ
EVENT_TYPE_DATA_WRITE_REQ = Manager_Events.EVENT_TYPE_DATA_WRITE_REQ
EVENT_TYPE_CONNECTION = Manager_Events.EVENT_TYPE_CONNECTION
EVENT_TYPE_STATUS = Manager_Events.EVENT_TYPE_STATUS
EVENT_TYPE_ENABLE = Manager_Events.EVENT_TYPE_ENABLE

LEGACY_PLC_NAME = "PLC1"  # what a 0.1.0 script gets from Manager() with no name
# Where 0.1.0 stored its connection, as framework option keys. Read, never written.
_LEGACY_SETTINGS = {"PLC_IP_ADDRESS": "br:Host", "PLC_PORT": "br:Port",
                    "REFRESH_RATE": "bridge:RefreshRate", "ENABLE_COMMUNICATION": "bridge:Enable"}


class _LegacyPlc:
    """
    Keeps the in-memory PLC1 of a no-name Manager() alive. Any rescan (stage
    open, the window's Refresh, a driver registered, a reload) drops components
    without a prim; it is re-created on the next app update with the options it
    last had. A PLC1 built from a prim is left alone.
    """

    def __init__(self, system):
        settings = carb.settings.get_settings()
        self._options = {key: settings.get("/persistent/loupe.simulation.br_bridge/" + old)
                         for old, key in _LEGACY_SETTINGS.items()}
        self._runtime = self._create(system, "created from the 0.1.0 persistent settings")
        self._sub = omni.kit.app.get_app().get_update_event_stream().create_subscription_to_pop(
            self._on_update, name="loupe.simulation.br_bridge.legacy_plc")

    def _create(self, system, why):
        system.add_component(LEGACY_PLC_NAME, dict(self._options, **{"bridge:driver": DRIVER_NAME}), author_prim=False)
        logger.warning("BrBridge.Manager() without a name: no '%s%s' prim, so '%s' was %s (%s). Add the prim with "
                       "bridge:driver = \"br\" and call Manager('%s').", system.system_root, LEGACY_PLC_NAME,
                       LEGACY_PLC_NAME, why, self._options, LEGACY_PLC_NAME)
        return system.get_component(LEGACY_PLC_NAME)

    def _on_update(self, _event):
        system = get_system()
        if system is None or registry.get(DRIVER_NAME) is None:
            return
        runtime = system.get_component(LEGACY_PLC_NAME)
        if runtime is not None:
            if runtime is self._runtime:
                self._options = runtime.options
            return
        try:
            self._runtime = self._create(system, "re-created in memory after a rescan dropped it")
        except Exception:
            logger.exception("BrBridge: could not re-create the in-memory '%s'; giving up", LEGACY_PLC_NAME)
            self._sub = None

    def release(self):
        self._sub = None


_legacy_plc = None


def _release():
    """Stop keeping PLC1 alive. The extension's shutdown."""
    global _legacy_plc
    if _legacy_plc is not None:
        _legacy_plc.release()
        _legacy_plc = None


class Manager(_Manager):
    """
    The framework's `Manager` on the legacy bus names. `Manager()` with no name
    is the 0.1.0 form: DEPRECATED, removed in 0.4.0. It addresses PLC1 and,
    when no such PLC is loaded, keeps one in memory from the 0.1.0 settings.
    """

    def __init__(self, Name=None, name=None):
        Name = Name if Name is not None else name
        if Name is None:
            warnings.warn("BrBridge.Manager() without a PLC name is DEPRECATED and will be removed in 0.4.0; "
                          "pass the name of the PLC prim, e.g. Manager('PLC1')", DeprecationWarning, stacklevel=2)
            Name = LEGACY_PLC_NAME
            global _legacy_plc
            system = get_system()
            if _legacy_plc is None and system is not None and system.get_component(Name) is None:
                _legacy_plc = _LegacyPlc(system)
        super().__init__(Name, namespace=LEGACY_NAMESPACE)
