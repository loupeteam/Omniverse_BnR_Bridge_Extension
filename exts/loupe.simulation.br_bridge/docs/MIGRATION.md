# Migrating from 0.1.0 to 0.3.0

0.3.0 moves the PLC connection out of the app's persistent settings and into the USD stage, lets one stage talk to several PLCs, and puts the extension on the same shared runtime as the Beckhoff bridge. Existing scripts keep running (see [What still works](#what-still-works)), but the setup you did in the old window has to be redone once, as a prim.

There was no 0.2.x of this extension; the version jumps to line up with the Beckhoff bridge, whose 0.2.0 made the same change.

## What changed

| | 0.1.0 | 0.3.0 |
|---|---|---|
| Where the connection is configured | `B&R Bridge / Open Bridge Settings` window; `Save` wrote to the app's persistent settings | A prim under `/PLC/` in the stage, e.g. `/PLC/PLC1`, carrying `br_bridge:*` attributes; saved with the stage |
| Number of PLCs | One | Any number, one prim each |
| Python API | `Manager()` | `Manager("PLC1")`, the name of the prim under `/PLC/` |
| Values read from the PLC | Only delivered to Python callbacks | Also mirrored into the stage as prims below the PLC prim (session layer), with `write:*` attributes for writing back without any Python |
| Menu entry | `B&R Bridge / Open Bridge Settings` | `Loupe / B&R Bridge` |
| Window buttons | `Save` / `Load` (persistent settings) | `Write To USD` / `Update From USD` (the PLC prim) |
| Headless apps | Nothing connected until the window was opened | Runtimes are created at startup and on every stage open |
| Driver | `websockets_driver.py` inside the extension, `async` | `br_bridge.BrDriver`, a plain-Python package at the repo root, usable without Omniverse |
| Threads | One thread running an asyncio loop that read, wrote and updated the UI | One daemon worker per PLC (`plc_bridge.PlcRuntime`); the websocket loop is the driver's own daemon thread |

## Step 1: put the connection in the stage

Open the stage you use with the PLC, then either:

- **From the window.** `Loupe / B&R Bridge`, type a name in `Add component` (use `PLC1` if your scripts call `Manager()` with no name), click `Add`. Fill in the IP address, port, refresh rate and the cyclic read variables, tick `Enable Client`, then click `Write To USD`. Save the stage.

- **By hand in the `.usda`.** Add a prim with these attributes anywhere under `/PLC/`:

  ```usda
  def Scope "PLC"
  {
      def Scope "PLC1"
      {
          custom string br_bridge:Host = "127.0.0.1"
          custom int br_bridge:Port = 8000
          custom bool br_bridge:Enable = true
          custom int br_bridge:RefreshRate = 20
          custom string br_bridge:Variables = "TestProg:counter,TestProg:structOfStructs.var1"
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

## Step 2: name the PLC in your scripts

```python
# 0.1.0
br_bridge = BrBridge.Manager()

# 0.3.0
br_bridge = BrBridge.Manager("PLC1")
```

Everything else in the script stays the same: `register_init_callback`, `register_data_callback`, `add_cyclic_read_variables`, `write_variable`, and the `event.payload["data"]` shape in the data callback are unchanged. `write_variables(dict)` is new for writing several values in one request.

## Step 3 (optional): drop the Python

If a script only existed to copy PLC values onto prims, or to write a value when something in the scene changed, it may not be needed any more. Every variable read from the PLC appears as a prim below the PLC prim, following the variable's structure (`TestProg:structOfStructs.var1` becomes `/PLC/PLC1/TestProg/structOfStructs/var1`; an array element `TestProg:arr[0]` becomes `/PLC/PLC1/TestProg/arr/_0`), with `value`, `write:value`, `write:pause` and `write:once` attributes. OmniGraph and the property window can read and write those directly. See the README for details.

## What still works

- **`Manager()` with no name.** Deprecated, removal planned for 0.4.0. It addresses `PLC1`. If no `/PLC/PLC1` prim is loaded it creates that runtime in memory from the old persistent settings and logs a warning, so an unchanged 0.1.0 script on an unchanged machine still connects. The runtime is not saved with the stage and disappears when the stage closes, until the next `Manager()` call. Do Step 1 to make it permanent.
- **Data callbacks.** Same payload shape; a `meta` key with the PLC name is added.
- **Init callbacks.** Still called once immediately with `None` and again on each init event. Note that an init event's payload no longer carries an empty `data` key.
- **Variable names.** `Task:var.member[i]` as before; the parser is now the shared `plc_bridge.nest_symbol` with the same `":."` separators, and the 0.1.0 parser tests run against it unchanged (`br_bridge/tests/test_symbols_legacy.py`).

## What breaks

- **Subscribing to the message bus directly** with the exported `EVENT_TYPE_*` constants, instead of through `Manager`. The constants are now plain strings and the events are per PLC (`loupe.simulation.br_bridge.DATA_READ.PLC1`). Use `Manager` or `loupe.simulation.common.RuntimeBase.get_stream_name(EVENT_TYPE_DATA_READ, "PLC1")` to build the event id.
- **Importing `loupe.simulation.br_bridge.websockets_driver`.** The module is gone. Use `br_bridge.BrDriver` (synchronous, implements the `plc_bridge.PlcDriver` contract) or, from inside Kit, `get_system().get_component("PLC1").plc` for the running runtime.
- **The old persistent settings are no longer written.** The window edits the PLC prim instead.
- **The `Dev Tools` section** of the window (latency fields, `Add variables for test program`, ad-hoc read and write fields) is gone. Use the `Cyclic Read Variables` field and the mirror prims' `write:value` instead.
- **Kit older than 105.** The shared code uses Python 3.10 syntax.

## Status messages

The `Status` field wording changed. Old and new:

| 0.1.0 | 0.3.0 |
|---|---|
| `Disabled` | (no message; the `Enable Client` box is unticked) |
| `Connecting...` | `Connecting` |
| `Connected` | `Connected` |
| `Connection Refused Error, check IP and Port: ...`, `Connection Error: ...` | `Error Connecting: ...` |
| `Connection Closed Error: ...` | `Disconnected` |
| `PLC read data parsing error: ...` | `Error Reading: ...` (a symbol the PLC returns as `undefined` or leaves out is named: `Error Reading: <symbol>: undefined`), then `Reading OK` when it clears |
| `Error writing data to PLC: ...` | `Error Writing: ...` |
