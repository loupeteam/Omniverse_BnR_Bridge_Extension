"""
  File: **BrBridge.py**
  Copyright (c) 2024 Loupe
  https://loupe.team

  This file is part of Omniverse_BnR_Bridge_Extension, licensed under the MIT License.

DEPRECATED compatibility module. The script API moved to the framework
extension `loupe.simulation.bridge`; this module re-exports it on the 0.1.0
bus names (`loupe.simulation.br_bridge.<KIND>.<plc>`) so existing scripts run
unchanged. Timeline: 0.3 warns, 0.4 turns the legacy bus names off by default
(the framework setting `legacyBusNames`), 0.5 removes this module.

    from loupe.simulation.bridge import Manager, get_plc, on_sample_main   # instead
"""

import logging
import warnings

import carb.settings
from loupe.simulation.bridge import Manager as _Manager
from loupe.simulation.bridge import Manager_Events as _ManagerEvents
from loupe.simulation.bridge import get_system  # noqa: F401  re-exported
from loupe.simulation.bridge.bus import get_stream_name  # noqa: F401  re-exported

from .extension import DRIVER_NAME, LEGACY_NAMESPACE

logger = logging.getLogger(__name__)

_MESSAGE = (
    "loupe.simulation.br_bridge.BrBridge is DEPRECATED and will be removed in 0.5: import Manager, "
    "get_system, get_plc and on_sample_main from loupe.simulation.bridge. Its bus names "
    "(loupe.simulation.br_bridge.*) are off by default from 0.4."
)
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

# The PLC a 0.1.0 script gets when it calls Manager() with no name.
LEGACY_PLC_NAME = "PLC1"

# Where 0.1.0 stored its single connection, mapped to the framework's option
# keys. Read once, only to seed the in-memory PLC; nothing is written back.
_LEGACY_SETTINGS = {
    "PLC_IP_ADDRESS": "br:Host",
    "PLC_PORT": "br:Port",
    "REFRESH_RATE": "bridge:RefreshRate",
    "ENABLE_COMMUNICATION": "bridge:Enable",
}


def _legacy_options() -> dict:
    """The 0.1.0 persistent settings that were actually saved, as component options."""
    settings = carb.settings.get_settings()
    options = {"bridge:driver": DRIVER_NAME}
    for old_key, key in _LEGACY_SETTINGS.items():
        value = settings.get("/persistent/loupe.simulation.br_bridge/" + old_key)
        if value is not None:
            options[key] = value
    return options


def _ensure_legacy_plc(system) -> None:
    """
    Create PLC1 in memory when no PLC of that name is loaded, seeded from the
    0.1.0 persistent settings. Nothing is authored into the user's file, so the
    runtime goes when the stage closes, until the next Manager() call.
    """
    if system is None or system.get_component(LEGACY_PLC_NAME) is not None:
        return
    options = _legacy_options()
    system.add_component(LEGACY_PLC_NAME, options, author_prim=False)
    logger.warning(
        "BrBridge.Manager() was called without a PLC name and no '%s%s' prim is loaded, so a "
        "'%s' runtime was created in memory from the 0.1.0 persistent settings (%s). Add a "
        "'%s%s' prim with bridge:driver = \"br\" and call Manager('%s').",
        system.system_root, LEGACY_PLC_NAME, LEGACY_PLC_NAME, options,
        system.system_root, LEGACY_PLC_NAME, LEGACY_PLC_NAME,
    )


class Manager(_Manager):
    """
    The framework's `Manager` on the 0.1.0 bus names. `Manager()` with no name
    is the 0.1.0 form: DEPRECATED, removed in 0.4.0. It addresses PLC1 and,
    when no such PLC is loaded, creates it in memory from the 0.1.0 settings.
    """

    def __init__(self, Name=None):
        if Name is None:
            warnings.warn("BrBridge.Manager() without a PLC name is DEPRECATED and will be removed "
                          "in 0.4.0; pass the name of the PLC prim, e.g. Manager('PLC1')",
                          DeprecationWarning, stacklevel=2)
            Name = LEGACY_PLC_NAME
            _ensure_legacy_plc(get_system())
        super().__init__(Name, namespace=LEGACY_NAMESPACE)
