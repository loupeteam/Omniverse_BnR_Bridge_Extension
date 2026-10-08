"""
Kit-side tests for the B&R extension.

The driver is plain Python with its own pytest suite under br_bridge/tests at
the repo root; the framework (prims, System, bus, mirror) is tested in
Omni-Utils. What is checked here is what this extension adds: the driver
registration and the deprecated BrBridge compatibility module.
"""

import warnings

import omni.kit.test
import omni.usd


class TestBrExtension(omni.kit.test.AsyncTestCase):

    async def test_driver_registered(self):
        from br_bridge import BrDriver
        from loupe.simulation.bridge import registry

        spec = registry.get("br")
        self.assertIsNotNone(spec, "the extension did not register the 'br' driver")
        self.assertIs(spec.driver_class, BrDriver)
        self.assertEqual(spec.legacy_namespace, "br_bridge")
        self.assertEqual(spec.attribute("Host"), "br:Host")
        self.assertEqual(spec.defaults, {"Host": "127.0.0.1", "Port": 8000})
        driver = spec.create_driver({"Host": "10.0.0.2", "Port": "8001"})
        self.assertEqual((driver.host, driver.port), ("10.0.0.2", 8001))

    async def test_compat_module(self):
        import importlib
        import sys

        sys.modules.pop("loupe.simulation.br_bridge.BrBridge", None)
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            BrBridge = importlib.import_module("loupe.simulation.br_bridge.BrBridge")
        self.assertTrue(any(issubclass(w.category, DeprecationWarning) for w in caught))
        self.assertEqual(BrBridge.EVENT_TYPE_DATA_READ, "loupe.simulation.br_bridge.DATA_READ")
        from loupe.simulation.bridge import Manager, get_system

        self.assertIs(BrBridge.get_system, get_system)
        self.assertTrue(issubclass(BrBridge.Manager, Manager))

    async def test_no_name_manager_creates_plc1_in_memory(self):
        await omni.usd.get_context().new_stage_async()
        from loupe.simulation.br_bridge import BrBridge

        system = BrBridge.get_system()
        self.assertIsNone(system.get_component("PLC1"))
        with warnings.catch_warnings(record=True) as caught:
            warnings.simplefilter("always")
            manager = BrBridge.Manager()
        try:
            self.assertTrue(any(issubclass(w.category, DeprecationWarning) for w in caught))
            runtime = system.get_component("PLC1")
            self.assertIsNotNone(runtime)
            self.assertEqual(runtime.driver_name, "br")
            prim = omni.usd.get_context().get_stage().GetPrimAtPath("/PLC/PLC1")
            self.assertFalse(prim and prim.IsValid(), "the legacy PLC must not be authored")
        finally:
            manager.cleanup()
            system.remove_component("PLC1")
