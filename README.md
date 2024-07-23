# Info
This tool is provided by Loupe.  
https://loupe.team  
info@loupe.team  
1-800-240-7042

# Description

This is an extension that connects B&R PLCs into the Omniverse ecosystem. It leverages [websockets](https://github.com/python-websockets/websockets) and the [OMJSON](https://github.com/loupeteam/OMJSON) library for B&R PLCs.

# Documentation

Detailed documentation can be found in the extension readme file [here](exts/loupe.simulation.br_bridge/docs/README.md).

# Testing

To run this extension's unit tests, open Omniverse's Extensions Manager, go to the Tests tab, and click "Run Extension Tests." Enabling UI Mode makes it easier to perform individual tests.

# Licensing

This software contains source code provided by NVIDIA Corporation. This code is subject to the terms of the [NVIDIA Omniverse License Agreement](https://docs.omniverse.nvidia.com/isaacsim/latest/common/NVIDIA_Omniverse_License_Agreement.html). Files are licensed as follows:

### Files created entirely by Loupe ([MIT License](LICENSE)):
* `websockets_driver.py`
* `br_bridge.py`

### Files including Nvidia-generated code and modifications by Loupe (Nvidia Omniverse License Agreement AND MIT License; use must comply to whichever is most restrictive for any attribute):
* `__init__.py`
* `extension.py`
* `global_variables.py`
* `ui_builder.py`

This software is intended for use with NVIDIA Omniverse apps, which are subject to the [NVIDIA Omniverse License Agreement](https://docs.omniverse.nvidia.com/isaacsim/latest/common/NVIDIA_Omniverse_License_Agreement.html) for use and distribution.
