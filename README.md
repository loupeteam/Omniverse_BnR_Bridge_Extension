# Info
This tool is provided by Loupe.  
https://loupe.team  
info@loupe.team  
1-800-240-7042

# Description

This is an extension that connects B&R PLCs into the Omniverse ecosystem. It leverages [websockets](https://github.com/python-websockets/websockets) and the [OMJSON](https://github.com/loupeteam/OMJSON) library for B&R PLCs.

# Documentation

Detailed documentation can be found in the extension readme file [here](exts/loupe.simulation.br_bridge/docs/README.md).

The extension is a thin driver for the vendor-neutral PLC bridge framework `loupe.simulation.bridge` from [Omni-Utils](https://github.com/loupeteam/Omni-Utils), which it depends on: the framework owns the PLC prims, the window, the Python API, the message bus and the USD mirror, and this extension registers the B&R driver with it. The driver itself is the plain-Python [`br_bridge`](br_bridge/README.md) package and can be used without Omniverse; it implements the `plc_bridge` driver contract, also from Omni-Utils. There are no submodules: the framework is an extension dependency, and the libraries are pip requirements. The design is laid out in the Beckhoff repo's `docs/ARCHITECTURE_PLAN.md`.

# Working from a clone

Until the packages are on PyPI, each extension installs its pip requirements from its own `wheels/` folder, and both folders have to be filled. Kit's pipapi looks only in the archive folders of the extensions started so far and then on PyPI, so the framework, which starts first, cannot find `plc-bridge` in this extension's folder. Once, and again after a version bump:

```
python <Omni-Utils checkout>/tools/build_wheels.py                                   # the framework: plc-bridge
python tools/build_wheels.py --plc-bridge <Omni-Utils checkout>/plc_bridge          # this extension: br-bridge, plc-bridge, websockets
```

Or run from source: Omni-Utils `tools/dev_link.py <kit build root> --driver <this repo>/br_bridge` installs both checkouts editable into Kit's Python, and Kit's pipapi then skips the wheels of both extensions (it imports `plc_bridge.runtime` and `br_bridge.driver` before it calls pip). Either way, add both repos' `exts` folders to the app's extension search paths.

The `wheels/` folder holds the websockets wheel built for the Python that ran the script plus pure-Python websockets wheels (the same version, and the newest one for Python 3.10), so it installs on Kit 105 and later on any platform.

`tools/kit_check` and `tools/kit_test.ps1` skip the framework's build step on purpose: they name this extension's `wheels/` folder as an app-wide pip archive (`/exts/omni.kit.pipapi/archiveDirs`), so the framework installs `plc-bridge` from it too. A normal app does not do that.

# Upgrading from 0.1.0

Version 0.3.0 moves the PLC connection into the USD stage and supports several PLCs. Existing scripts keep working, but the connection has to be set up once as a prim. See the [migration guide](exts/loupe.simulation.br_bridge/docs/MIGRATION.md).

# Testing

- `br_bridge/`: `pip install -e <Omni-Utils>/plc_bridge -e ./br_bridge[test]`, then `pytest` in `br_bridge/` (driver tests against an in-process mock OMJSON server, and the parser tests).
- Kit tests: `tools/kit_test.ps1 -Kit <kit build root> -BridgeExts <Omni-Utils>/exts`, or the Tests tab of the Extensions Manager.
- Headless, end to end: `tools/kit_check` (see its README), against a mock OMJSON server by default, or with `-Mode arsim` against `test/AS Project` running in ARsim.
- PLC side: `test/AS Project` is an Automation Studio 6 project (AR 6.7.6, OMJSON 2.0.0) that serves the test variables on `ws://127.0.0.1:8000`; its README says how to build it and run it in ARsim. It replaced the Automation Studio 4.10 project in 0.3.0, which is no longer in the repo.

# Licensing

This software contains source code provided by NVIDIA Corporation. This code is subject to the terms of the [NVIDIA Omniverse License Agreement](https://docs.omniverse.nvidia.com/isaacsim/latest/common/NVIDIA_Omniverse_License_Agreement.html). Files are licensed as follows:

### Files created entirely by Loupe ([MIT License](LICENSE)):
* everything under `br_bridge/` (the OMJSON driver as a plain Python package)
* `BrBridge.py`
* `tests/tests.py`
* everything under `tools/`

### Files including Nvidia-generated code and modifications by Loupe (Nvidia Omniverse License Agreement AND MIT License; use must comply to whichever is most restrictive for any attribute):
* `__init__.py`
* `extension.py`

This software is intended for use with NVIDIA Omniverse apps, which are subject to the [NVIDIA Omniverse License Agreement](https://docs.omniverse.nvidia.com/isaacsim/latest/common/NVIDIA_Omniverse_License_Agreement.html) for use and distribution.

The framework extension `loupe.simulation.bridge` and the `plc_bridge` package it depends on are part of [Omni-Utils](https://github.com/loupeteam/Omni-Utils) and carry their own licence terms; they are no longer vendored here.

This software also relies on [websockets](https://github.com/python-websockets/websockets), which is licensed under the BSD 3-Clause license.
