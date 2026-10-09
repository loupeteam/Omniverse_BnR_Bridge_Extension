# Migrating from 0.1.0 to 0.3.0

0.3.0 moves the PLC connection out of the app's persistent settings and into the USD stage, lets one stage talk to several PLCs, and turns this extension into a driver for the vendor-neutral PLC bridge framework, `loupe.simulation.bridge` (from [Omni-Utils](https://github.com/loupeteam/Omni-Utils)), which the Beckhoff bridge uses too. Existing scripts keep running with deprecation warnings (see [What still works](#what-still-works)), but the setup you did in the old window has to be redone once, as a prim.

There was no 0.2.x of this extension; the version jumps to line up with the Beckhoff bridge.

## What changed

| | 0.1.0 | 0.3.0 |
|---|---|---|
| Where the connection is configured | `B&R Bridge / Open Bridge Settings` window; `Save` wrote to the app's persistent settings | A prim under `/PLC/` in the stage, e.g. `/PLC/PLC1`, with `bridge:driver = "br"`, `br:Host`, `br:Port` and the `bridge:*` settings; saved with the stage |
| Number of PLCs | One | Any number, one prim each, Beckhoff and B&R mixed |
| Who does the work | This extension | The framework `loupe.simulation.bridge`, enabled with this one; this extension registers the B&R driver |
| Python API | `from loupe.simulation.br_bridge import BrBridge`, `BrBridge.Manager()` | `from loupe.simulation.bridge import Manager, get_plc, on_sample_main`; `Manager("PLC1")` takes the name of the prim under `/PLC/` |
| Message bus events | `loupe.simulation.br_bridge.DATA_READ`, one stream | `loupe.simulation.bridge.<KIND>.<plc>`, one stream per PLC |
| Values read from the PLC | Only delivered to Python callbacks | Also mirrored into the stage as prims below the PLC prim (session layer), with `write:*` attributes for writing back without any Python; can be switched off |
| Menu entry | `B&R Bridge / Open Bridge Settings` | `Loupe / PLC Bridge` |
| Window buttons | `Save` / `Load` (persistent settings) | `Write To USD` / `Update From USD` (the PLC prim) |
| Headless apps | Nothing connected until the window was opened | Runtimes are created at startup and on every stage open |
| Driver | `websockets_driver.py` inside the extension, `async` | `br_bridge.BrDriver`, a plain-Python package (`br-bridge`), usable without Omniverse |
| Threads | One thread running an asyncio loop that read, wrote and updated the UI | One daemon worker per PLC (`plc_bridge.PlcRuntime`); the websocket loop is the driver's own daemon thread |

## Step 1: put the connection in the stage

Open the stage you use with the PLC, then either:

- **From the window.** `Loupe / PLC Bridge`, type a name in `Add component` (use `PLC1` if your scripts call `Manager()` with no name), click `Add`. Fill in the IP address, port, refresh rate and the variables, tick `Enable`, then click `Write To USD`. Save the stage.

- **By hand in the `.usda`.** Add a prim anywhere under `/PLC/`:

  ```usda
  def Scope "PLC"
  {
      def Scope "PLC1"
      {
          custom string   bridge:driver      = "br"
          custom bool     bridge:Enable      = true
          custom int      bridge:RefreshRate = 20
          custom string[] bridge:Variables   = ["TestProg:counter", "TestProg:structOfStructs.var1"]
          custom string   br:Host            = "127.0.0.1"
          custom int      br:Port            = 8000
      }
  }
  ```

The values to carry over are the ones you last saved in the old window. They are still in the app's persistent settings under `/persistent/loupe.simulation.br_bridge/` as `PLC_IP_ADDRESS`, `PLC_PORT`, `REFRESH_RATE` and `ENABLE_COMMUNICATION`; the Kit `Preferences` window shows them, or a script can read them:

```python
import carb.settings
s = carb.settings.get_settings()
print(s.get("/persistent/loupe.simulation.br_bridge/PLC_IP_ADDRESS"))
```

Variables were never persisted in 0.1.0, so they come from your script's `add_cyclic_read_variables` call; you can leave them there or move them onto the prim.

0.3.0rc1 wrote the vendor-named form (`br_bridge:Host`, `br_bridge:Port`, `br_bridge:Enable`, `br_bridge:RefreshRate`, `br_bridge:Variables` as a comma-separated string, no `bridge:driver`). Such a prim still loads, with a warning; `Write To USD` rewrites it in the form above.

## Step 2: import from the framework and name the PLC

```python
# 0.1.0
from loupe.simulation.br_bridge import BrBridge
br_bridge = BrBridge.Manager()

# 0.3.0
from loupe.simulation.bridge import Manager
br_bridge = Manager("PLC1")
```

Everything else in the script stays the same: `register_init_callback`, `register_data_callback`, `add_cyclic_read_variables`, `write_variable`, and the `event.payload["data"]` shape in the data callback are unchanged. `write_variables(dict)` is new for writing several values in one request.

Code that touches the stage or the UI should prefer `on_sample_main("PLC1", callback)`: it runs on the main thread once per app update with the newest sample, where bus callbacks run on the PLC's worker thread. The framework's `docs/CONSUMING.md` has the options and the thread rules.

## Step 3 (optional): drop the Python, or the mirror

If a script only existed to copy PLC values onto prims, or to write a value when something in the scene changed, it may not be needed any more. Every variable read from the PLC appears as a prim below the PLC prim, following the variable's structure (`TestProg:structOfStructs.var1` becomes `/PLC/PLC1/TestProg/structOfStructs/var1`; an array element `TestProg:arr[0]` becomes `/PLC/PLC1/TestProg/arr/_0`), with `value`, `symbol`, `write:value`, `write:pause` and `write:once` attributes. OmniGraph and the property window can read and write those directly.

The mirror is on by default in 0.3 and costs a USD write per value per frame. If nothing in your app reads it, switch it off per PLC with `custom bool bridge:MirrorToUsd = false` on the prim, or mirror only what you need with `custom string[] bridge:MirrorSymbols = ["TestProg:counter"]`. From 0.4 the mirror is off by default; set `bridge:MirrorToUsd = true` on prims that rely on it.

## What still works, and until when

The vendor-named surfaces are a one-release alias. Each logs a deprecation warning that names its replacement.

| Old surface | 0.3 | 0.4 | 0.5 |
|---|---|---|---|
| `from loupe.simulation.br_bridge import BrBridge` (`Manager`, `get_system`, `EVENT_TYPE_*`) | works, `DeprecationWarning` at import | still there, `DeprecationWarning` at import; its `Manager` talks on the neutral names unless `legacyBusNames` is turned back on | removed |
| Bus names `loupe.simulation.br_bridge.<KIND>.<plc>` | pushed and accepted next to the neutral names | only with the framework setting `/exts/loupe.simulation.bridge/legacyBusNames = true` | removed |
| Prim attributes `br_bridge:*` without `bridge:driver` | read, warned once per prim | only behind a setting | removed |
| `BrBridge.Manager()` with no name | works, warns | removed (announced in 0.3.0rc1) | |

- **`Manager()` with no name** addresses `PLC1`. If no `/PLC/PLC1` prim is loaded it creates that runtime in memory from the old persistent settings and logs a warning, so an unchanged 0.1.0 script on an unchanged machine still connects. The runtime is never saved with the stage. Whenever the framework rescans (a stage is opened, `Refresh` in the window, a driver extension is enabled or reloaded) it drops runtimes without a prim; the compatibility module then re-creates `PLC1` on the next app update with the settings it last had (address, enable, refresh rate, variables) and logs that it did. The script's init callback runs again, as it did on every 0.1.0 init event. A `/PLC/PLC1` prim in the opened stage takes precedence. Do Step 1 to make it permanent.
- **Data callbacks.** Same payload shape; a `meta` key with the PLC name is added.
- **Init callbacks.** Still called once immediately with `None` and again when the PLC's runtime is created.
- **Variable names.** `Task:var.member[i]` as before; the parser is the shared `plc_bridge.nest_symbol` with the same `":."` separators, and the 0.1.0 parser tests run against it unchanged (`br_bridge/tests/test_symbols_legacy.py`).

## What breaks

- **Subscribing to the message bus directly** with the `EVENT_TYPE_*` constants, instead of through `Manager`. The events are per PLC: `loupe.simulation.bridge.DATA_READ.PLC1`. Build the id with `loupe.simulation.bridge.bus.get_stream_name(EVENT_TYPE_DATA_READ, "PLC1")`.
- **`DATA_INIT` payloads** carry `{"meta": {"name": "PLC1"}}` instead of 0.1.0's `{"data": {}}`. An init callback that read `event.payload["data"]` fails; one that only adds variables (the documented use) is unaffected.
- **`STATUS` payloads** on the neutral names carry a structured problem, `{"kind", "text", "symbols"}`; the legacy names still carry the text.
- **Importing `loupe.simulation.br_bridge.websockets_driver`** or any other module of the extension besides `BrBridge`. They are gone. Use `br_bridge.BrDriver` (synchronous, implements the `plc_bridge.PlcDriver` contract) or, from inside Kit, `loupe.simulation.bridge.get_plc("PLC1")` for the running runtime.
- **The old persistent settings are no longer written.** The window edits the PLC prim instead.
- **The `Dev Tools` section** of the window (latency fields, `Add variables for test program`, ad-hoc read and write fields) is gone. Use the variables field and the mirror prims' `write:value` instead.
- **Kit older than 105.** The code uses Python 3.10 syntax. On Kit 105 to 108 (Python 3.10) pip takes the newest websockets that supports 3.10 (16.x) from the bundled pure-Python wheels; later Kits take the current one.

## Status messages

| 0.1.0 | 0.3.0 |
|---|---|
| `Disabled` | (no message; the `Enable` box is unticked) |
| `Connecting...` | `Connecting` |
| `Connected` | `Connected` |
| `Connection Refused Error, check IP and Port: ...`, `Connection Error: ...` | `Error Connecting: ...` |
| `Connection Closed Error: ...` | `Disconnected` |
| `PLC read data parsing error: ...` | `Error Reading: ...` (a symbol the PLC returns as `undefined` or leaves out is named: `Error Reading: <symbol>: undefined`), then `Reading OK` when it clears |
| `Error writing data to PLC: ...` | `Error Writing: ...` |
