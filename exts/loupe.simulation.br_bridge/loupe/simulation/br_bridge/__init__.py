# This software contains source code provided by NVIDIA Corporation.
# Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
#
# Modifications copyright (c) 2024 Loupe, https://loupe.team, MIT License.

"""
loupe.simulation.br_bridge registers the B&R OMJSON driver with the
loupe.simulation.bridge framework. Everything else (PLC prims, the window,
the message bus, the USD mirror) belongs to the framework.

`BrBridge` is a deprecated compatibility module for 0.1.0 scripts; new code
imports from `loupe.simulation.bridge`.
"""

import sys as _sys

# Kit has its working directory on sys.path, so from a checkout of this repo the
# bare br_bridge/ folder imports as an empty namespace package and pipapi's import
# check caches it before the wheel is installed. Drop it (no __file__).
_mod = _sys.modules.get("br_bridge")
if _mod is not None and getattr(_mod, "__file__", None) is None:
    del _sys.modules["br_bridge"]
del _mod, _sys

from .extension import *  # noqa: E402,F401,F403
