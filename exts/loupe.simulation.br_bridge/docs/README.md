# B&R Bridge

The B&R Bridge connects [NVIDIA Omniverse](https://www.nvidia.com/en-us/omniverse/) to [B&R PLCs](https://www.br-automation.com/) using websockets and the [OMJSON](https://github.com/loupeteam/OMJSON) library. The PLC runs OMJSON's `jsonWebSocketServer`; the bridge connects to it, reads a list of variables cyclically and writes values back.

Since 0.3.0 this extension is a thin driver for the vendor-neutral PLC bridge framework, [`loupe.simulation.bridge`](https://github.com/loupeteam/Omni-Utils) (from Omni-Utils), which it depends on and which Kit enables with it. The framework owns the PLC prims, the `PLC Bridge` window, the Python API, the message bus and the USD mirror; this extension registers the B&R driver, `br_bridge.BrDriver`, under the driver name `br`. The same simulation runs against a Beckhoff PLC by changing the driver attribute on the prim.

Upgrading from 0.1.0? See [MIGRATION.md](MIGRATION.md).

# Installation

### Install from registry

This is the preferred method. Open up the extensions manager by navigating to `Window / Extensions`. The extension is available as a "Third Party" extension. Search for `B&R Bridge`, and click the slider to enable it; Kit enables the `PLC Bridge` framework with it. The window is under `Loupe / PLC Bridge`.

### Install from source

- Clone this repo and [Omni-Utils](https://github.com/loupeteam/Omni-Utils) (the framework). No submodules any more.
- Until the packages are on PyPI, fill both extensions' `wheels/` folders once: `python <Omni-Utils>/tools/build_wheels.py` (the framework's `plc-bridge`) and `python tools/build_wheels.py --plc-bridge <Omni-Utils>/plc_bridge` (this extension's `br-bridge`, `plc-bridge`, `websockets`). Each extension installs only from its own folder, so skipping the first leaves the framework without `plc-bridge`. Alternatively, link the checkouts into Kit's Python with Omni-Utils `tools/dev_link.py <kit> --driver <this repo>/br_bridge`, which covers both.
- In your Omniverse app, open the extensions manager (`Window / Extensions`), open the general extension settings, and add both `exts` folders (this repo's and Omni-Utils') to `Extension Search Paths`.
- Search for `B&R BRIDGE` and enable it.

# Configuring a PLC

A PLC is a prim under `/PLC/` carrying `bridge:driver = "br"` and the B&R options under the `br:` namespace. The connection settings are saved with the stage, and a stage can describe any number of PLCs, of any registered vendor:

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

| Attribute | Type | Default | Meaning |
|---|---|---|---|
| `bridge:driver` | string | | `"br"` for this extension |
| `br:Host` | string | `127.0.0.1` | IP address of the PLC |
| `br:Port` | int | `8000` | Port of the OMJSON `jsonWebSocketServer` |
| `bridge:Enable` | bool | `false` | Enable or disable the client |
| `bridge:RefreshRate` | int | `20` | Cyclic read period in milliseconds |
| `bridge:Variables` | string[] | `[]` | Variables to read cyclically |
| `bridge:MirrorToUsd` | bool | `true` | Mirror the values as prims under the PLC prim |
| `bridge:MirrorSymbols` | string[] | `[]` | Mirror only these variables; empty means all |

The framework's README has the full schema, the `autoConnect` safety setting, and the mirror layout. Prims written by 0.3.0rc1 (`br_bridge:Host`, `br_bridge:Port`, `br_bridge:Enable`, `br_bridge:RefreshRate`, `br_bridge:Variables` as a comma-separated string) still load, with a deprecation warning; see [MIGRATION.md](MIGRATION.md).

Variable names follow OMJSON: `globalVar` for a global, `Task:localVar` for a task-local variable, with `.` for structure members and `[i]` for array elements, for example `TestProg:structOfStructs.secondStruct.array[2]`.

When the framework starts, and whenever a stage is opened or closed, every PLC prim becomes a running runtime: a daemon worker thread that connects, reads the variables at the refresh rate and delivers the values. No window has to be opened, so the bridge also works headless.

# The window

`Loupe / PLC Bridge` (the framework's window) lists the PLCs of the stage. For a B&R PLC it shows `PLC IP Address` and `PLC Port` below the common fields (`Enable`, `Refresh Rate (ms)`, the variables, `Write To USD` / `Update From USD`, connection and status).

Status messages: `Connecting`, `Connected`, `Disconnected`, `Error Connecting: [...]` (wrong IP or port, PLC offline, `jsonWebSocketServer` not running), `Error Reading: [...]` (a symbol the PLC does not know is named: `<symbol>: undefined`, or `<symbol>: not in response`; `Reading OK` follows when it recovers), `Error Writing: [...]`.

# Using PLC data from Python

Import from the framework, which is the same for every vendor; spell the variables as the PLC knows them (`TestProg:lreal`):

```python
from loupe.simulation.bridge import get_plc, on_sample_main

def on_plc(sample):                       # main thread, once per app update
    value = sample.values["TestProg:lreal"]

remove = on_sample_main("PLC1", on_plc)

plc = get_plc("PLC1")                     # the plc_bridge.PlcRuntime
plc.queue_write("TestProg:lreal", 2.5)
```

The message-bus `Manager` of 0.1.0 is there too, on the neutral bus names:

```python
from loupe.simulation.bridge import Manager

manager = Manager("PLC1")
manager.register_init_callback(lambda event: manager.add_cyclic_read_variables(["TestProg:counter"]))
manager.register_data_callback(lambda event: print(event.payload["data"]["TestProg"]["counter"]))
manager.write_variable("TestProg:counter", 1)
```

See the framework's `docs/CONSUMING.md` for which way to pick and the thread rules.

`from loupe.simulation.br_bridge import BrBridge` still works but is **deprecated**: it warns on import and serves `Manager`, `get_system` and the `EVENT_TYPE_*` constants on the legacy `br_bridge` namespace: per-PLC events named `loupe.simulation.br_bridge.<KIND>.<plc>`, which the framework pushes next to its neutral names. Off by default from 0.4, removed in 0.5. `Manager` takes the PLC name as `Manager("PLC1")`, `Manager(Name="PLC1")` or `Manager(name="PLC1")`.

# Testing

The driver is plain Python with a pytest suite under `br_bridge/tests` (see `br_bridge/README.md`). The Kit tests in this extension check the driver registration and the compatibility module (`tools/kit_test.ps1`). `tools/kit_check` runs the extension and the framework headlessly against a mock OMJSON server.
