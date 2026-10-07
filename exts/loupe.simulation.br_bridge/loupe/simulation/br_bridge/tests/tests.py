"""
Kit-side smoke tests for the B&R bridge extension.

The driver and the parser are plain Python and have their own pytest suite under
br_bridge/tests at the repo root. What is checked here is only what needs Kit:
that the extension loads, that the libraries the manifest points at are
importable inside Kit, and that the System is up with the B&R option keys.
"""

import omni.kit.test


class TestExtensionLoads(omni.kit.test.AsyncTestCase):

    async def test_libraries_importable_in_kit(self):
        """The [[python.module]] entries in extension.toml resolve inside Kit."""
        import plc_bridge
        import br_bridge
        from br_bridge import BrDriver
        from plc_bridge import PlcDriver, nest_symbol

        self.assertTrue(issubclass(BrDriver, PlcDriver))
        self.assertEqual(BrDriver.symbol_separators, ":.")
        self.assertEqual(
            nest_symbol({}, "TestProg:structOfStructs.var1", 3, ":."),
            {"TestProg": {"structOfStructs": {"var1": 3}}},
        )
        self.assertTrue(plc_bridge.__name__ and br_bridge.__name__)

    async def test_libraries_do_not_import_kit(self):
        """The plain-Python packages must stay free of omni/carb imports."""
        import os
        import br_bridge
        import plc_bridge

        for package in (br_bridge, plc_bridge):
            folder = os.path.dirname(package.__file__)
            for name in os.listdir(folder):
                if not name.endswith(".py"):
                    continue
                with open(os.path.join(folder, name), encoding="utf-8") as f:
                    source = f.read()
                for forbidden in ("import omni", "from omni", "import carb", "from carb"):
                    self.assertNotIn(forbidden, source, f"{package.__name__}/{name} imports Kit")

    async def test_system_is_created_with_br_options(self):
        from loupe.simulation.br_bridge.BrBridge import get_system, Manager_Events
        from loupe.simulation.br_bridge.global_variables import (
            ATTR_BR_BRIDGE_HOST,
            ATTR_BR_BRIDGE_PORT,
            ATTR_BR_BRIDGE_ENABLE,
            ATTR_BR_BRIDGE_REFRESH,
            ATTR_BR_BRIDGE_READ_VARS,
        )

        system = get_system()
        self.assertIsNotNone(system, "extension did not create its System on startup")
        self.assertEqual(system.system_root, "/PLC/")
        self.assertEqual(
            set(system.default_properties),
            {
                ATTR_BR_BRIDGE_HOST,
                ATTR_BR_BRIDGE_PORT,
                ATTR_BR_BRIDGE_ENABLE,
                ATTR_BR_BRIDGE_REFRESH,
                ATTR_BR_BRIDGE_READ_VARS,
            },
        )
        self.assertEqual(
            Manager_Events.EVENT_TYPE_DATA_READ, "loupe.simulation.br_bridge.DATA_READ"
        )
