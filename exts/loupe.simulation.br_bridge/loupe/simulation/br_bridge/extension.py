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
Extension entry point: register `br_bridge.BrDriver` with the framework's
driver registry on startup, unregister it on shutdown.

The framework then builds a runtime for every prim carrying
`bridge:driver = "br"` (options `br:Host`, `br:Port`) and, through the legacy
namespace, for every 0.3.0rc1 prim carrying `br_bridge:*` attributes, with a
deprecation warning. While its `legacyBusNames` setting is on it also pushes
and accepts the `loupe.simulation.br_bridge.*` bus names. Its window builds a
field per option from the schema below, so no UI panel is registered.
"""

import sys

import omni.ext
from br_bridge import BrDriver
from loupe.simulation.bridge import Option, check_extension_requirements, registry

DRIVER_NAME = "br"
LEGACY_NAMESPACE = "br_bridge"
TITLE = "B&R (OMJSON)"

OPTIONS = [Option("Host", "str", "127.0.0.1", "PLC IP Address"), Option("Port", "int", 8000, "PLC Port")]


class Extension(omni.ext.IExt):
    def on_startup(self, ext_id: str):
        # Kit's pip installer only checks that br_bridge imports; log an error
        # when the one it found is not the version this extension pins.
        check_extension_requirements(ext_id)
        registry.register(DRIVER_NAME, BrDriver, OPTIONS, legacy_namespace=LEGACY_NAMESPACE, title=TITLE)

    def on_shutdown(self):
        compat = sys.modules.get(__package__ + ".BrBridge")
        if compat is not None:
            compat._release()  # stop keeping a no-name Manager()'s PLC1 alive
        # Only our own entry: leave a driver someone else registered under the name.
        spec = registry.get(DRIVER_NAME)
        if spec is not None and spec.driver_class is BrDriver:
            registry.unregister(DRIVER_NAME)
