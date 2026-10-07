# Info
This tool is provided by Loupe.  
https://loupe.team  
info@loupe.team  
1-800-240-7042

# Description

This is an extension that connects B&R PLCs into the Omniverse ecosystem. It leverages [websockets](https://github.com/python-websockets/websockets) and the [OMJSON](https://github.com/loupeteam/OMJSON) library for B&R PLCs.

# Documentation

Detailed documentation can be found in the extension readme file [here](exts/loupe.simulation.br_bridge/docs/README.md).

The OMJSON driver itself is the plain-Python [`br_bridge`](br_bridge/README.md) package and can be used without Omniverse. It implements the vendor-neutral `plc_bridge` driver contract from [Omni-Utils](https://github.com/loupeteam/Omni-Utils), which the extension vendors as a submodule at `exts/loupe.simulation.br_bridge/loupe/simulation/common` (clone with `--recurse-submodules`). The direction for the next major version is laid out in the Beckhoff repo's `docs/ARCHITECTURE_PLAN.md`.

# Upgrading from 0.1.0

Version 0.3.0 moves the PLC connection into the USD stage and supports several PLCs. Existing scripts keep working, but the connection has to be set up once as a prim. See the [migration guide](exts/loupe.simulation.br_bridge/docs/MIGRATION.md).

# Testing

- `br_bridge/`: `pip install -e <Omni-Utils>/plc_bridge -e ./br_bridge[test]`, then `pytest` in `br_bridge/` (driver tests against an in-process mock OMJSON server, and the parser tests).
- Kit: open Omniverse's Extensions Manager, go to the Tests tab, and click "Run Extension Tests."
- Headless, end to end: `tools/kit_check` (see its README) against `test/AS Project` running in ARsim, or against the mock server.

# Licensing

This software contains source code provided by NVIDIA Corporation. This code is subject to the terms of the [NVIDIA Omniverse License Agreement](https://docs.omniverse.nvidia.com/isaacsim/latest/common/NVIDIA_Omniverse_License_Agreement.html). Files are licensed as follows:

### Files created entirely by Loupe ([MIT License](LICENSE)):
* everything under `br_bridge/` (the OMJSON driver as a plain Python package)
* `Runtime.py`
* `BrBridge.py`
* everything under `loupe/simulation/common` (the [Omni-Utils](https://github.com/loupeteam/Omni-Utils) submodule)
* everything under `tools/`

### Files including Nvidia-generated code and modifications by Loupe (Nvidia Omniverse License Agreement AND MIT License; use must comply to whichever is most restrictive for any attribute):
* `__init__.py`
* `extension.py`
* `global_variables.py`
* `ui_builder.py`

This software is intended for use with NVIDIA Omniverse apps, which are subject to the [NVIDIA Omniverse License Agreement](https://docs.omniverse.nvidia.com/isaacsim/latest/common/NVIDIA_Omniverse_License_Agreement.html) for use and distribution.

This software also relies on [websockets](https://github.com/python-websockets/websockets), which is licensed under the BSD 3-Clause license.
