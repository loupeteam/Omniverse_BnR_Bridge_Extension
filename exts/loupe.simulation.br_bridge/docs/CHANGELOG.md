Changelog

[0.3.0-rc1]
See MIGRATION.md for upgrading from 0.1.0. This release brings the B&R extension onto the same shared runtime as the Beckhoff one (Phase 2 of the Beckhoff repo's `docs/IMPLEMENTATION_PLAN.md`).
- Support multiple PLCs. Each PLC is a prim under `/PLC/` carrying `br_bridge:*` attributes (`Host`, `Port`, `Enable`, `RefreshRate`, `Variables`), so connection settings are saved in the stage instead of in persistent app settings.
- Mirror the values read from the PLC into the stage as prims in the session layer, with `write:*` attributes for writing back. Array elements are mirrored as `_<index>` child prims. Switch it off per PLC with `bridge:MirrorToUsd = false`.
- Create the runtimes at extension startup and on stage open/close, so the bridge works without opening the window (headless).
- The OMJSON websocket driver moved out of the extension into the plain-Python `br_bridge` package at the repo root (`br_bridge/`), with no Omniverse dependency and its own pytest suite. `websockets_driver.py` is gone; the parser it carried is now the shared `plc_bridge.nest_symbol` with the driver's separators `":."`, and its tests moved to `br_bridge/tests/test_symbols_legacy.py`.
- The polling (worker thread, connect and retry, read list, write queue, status reporting) is the vendor-neutral, plain-Python `plc_bridge.PlcRuntime` from the Omni-Utils submodule at `loupe/simulation/common`; `BrDriver` implements its `PlcDriver` contract. The extension's `Runtime` is an adapter between that runtime and the message bus. One daemon worker per PLC, each scan flushing the queued writes and then reading; a queued write wakes the loop.
- Message bus events are per PLC: `loupe.simulation.br_bridge.<KIND>.<name>`, with `CONNECTION`, `STATUS` and `ENABLE` added next to `DATA_INIT`, `DATA_READ`, `DATA_READ_REQ` and `DATA_WRITE_REQ`. Payloads carry a `meta.name` key.
- `BrBridge.Manager` now addresses one PLC by the name of its prim under `/PLC/` (`Manager("PLC1")`), gains `write_variables(dict)`, and a warning is logged when a `Manager` is created for a PLC that is not loaded. `BrBridge.get_system()` exposes the `System` that owns the runtimes.
- **Deprecated:** calling `Manager()` with no name (the 0.1.0 form). It still works: it addresses `PLC1` and, if no `/PLC/PLC1` prim is loaded, creates that runtime in memory from the 0.1.0 persistent settings (`PLC_IP_ADDRESS`, `PLC_PORT`, `REFRESH_RATE`, `ENABLE_COMMUNICATION`) with a warning. Nothing is written to the stage file, and the runtime is gone when the stage closes until the next `Manager()` call. Add a `/PLC/PLC1` prim to make it permanent. This path will be removed in 0.4.0.
- A symbol the PLC does not know (OMJSON returns `"undefined"`) or leaves out of its reply is no longer delivered as data; it is reported in the status once as `Error Reading: <symbol>: <reason>` and `Reading OK` when it recovers. A write the PLC rejects is reported as `Error Writing: <symbol>: <reason>`. Every request is bounded by the driver's timeout (3 s) and a lost websocket reports `Disconnected` and reconnects by itself.
- Menu entry moved from `B&R Bridge / Open Bridge Settings` to `Loupe / B&R Bridge`; the window shows one PLC at a time with `Write To USD` / `Update From USD` instead of `Save` / `Load`. The `Dev Tools` section (latency fields, test-program buttons) is gone.
- Declare the `omni.timeline`, `omni.usd` and `omni.kit.menu.utils` dependencies; drop the unused `omni.physx` dependency and the `asyncio` pip requirement (it is part of Python).
- Worker threads are daemons with a bounded join, so a runtime can no longer keep the app from exiting.
- Headless check harness under `tools/kit_check` (copied from the Beckhoff repo) with a `/PLC/PLC1` stage reading the `TestProg` symbols of `test/AS Project`.

[0.1.0]
- Created with based functionality to setup a connection and send/receive messages with other extensions.
