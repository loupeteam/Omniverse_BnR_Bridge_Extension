"""
Kit-side tests for the B&R extension.

The driver is plain Python with its own pytest suite under br_bridge/tests at
the repo root; the framework (prims, System, bus, mirror) is tested in
Omni-Utils. What is checked here is what this extension adds: the driver
registration and the deprecated BrBridge compatibility module.
"""

import importlib.util
import os
import time
import warnings

import omni.kit.test
import omni.usd

SETTINGS = "/persistent/loupe.simulation.br_bridge/"
REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), *[".."] * 6))


def _mock_module():
    """br_bridge/tests/mock_omjson.py from this checkout (not part of the wheel)."""
    spec = importlib.util.spec_from_file_location("mock_omjson", os.path.join(REPO, "br_bridge", "tests", "mock_omjson.py"))
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


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

    async def test_manager_follows_the_legacy_bus_setting(self):
        import carb.settings
        from loupe.simulation.br_bridge import BrBridge

        setting = "/exts/loupe.simulation.bridge/legacyBusNames"
        settings = carb.settings.get_settings()
        old = settings.get(setting)
        try:
            settings.set(setting, True)
            manager = BrBridge.Manager("PLC1")
            self.assertEqual(manager._events.EVENT_TYPE_DATA_READ, "loupe.simulation.br_bridge.DATA_READ")
            manager.cleanup()
            settings.set(setting, False)
            manager = BrBridge.Manager("PLC1")
            self.assertEqual(manager._events.EVENT_TYPE_DATA_READ, "loupe.simulation.bridge.DATA_READ")
            manager.cleanup()
            # The constants stay the legacy names either way, as in the Beckhoff module.
            self.assertEqual(BrBridge.EVENT_TYPE_DATA_READ, "loupe.simulation.br_bridge.DATA_READ")
        finally:
            settings.set(setting, True if old is None else old)

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
            layer = omni.usd.get_context().get_stage().GetRootLayer()
            self.assertIsNone(layer.GetPrimAtPath("/PLC/PLC1"), "the legacy PLC must not be authored")
        finally:
            manager.cleanup()
            BrBridge._release()
            system.remove_component("PLC1")

    async def test_no_name_manager_plc1_survives_rescans(self):
        """Stage open, the window's Refresh and a driver registration drop prim-less components."""
        import carb.settings
        import omni.kit.app
        from br_bridge import BrDriver
        from loupe.simulation.bridge import registry
        from loupe.simulation.br_bridge import BrBridge
        from loupe.simulation.br_bridge.extension import OPTIONS

        app = omni.kit.app.get_app()
        await omni.usd.get_context().new_stage_async()
        mock = _mock_module().MockOmjson({"TestProg:counter": 7}).start()
        settings = carb.settings.get_settings()
        keys = {"PLC_PORT": mock.port, "ENABLE_COMMUNICATION": True}
        old = {key: settings.get(SETTINGS + key) for key in keys}
        for key, value in keys.items():
            settings.set(SETTINGS + key, value)
        system = BrBridge.get_system()
        data = []
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            manager = BrBridge.Manager()
        manager.register_init_callback(lambda event: manager.add_cyclic_read_variables(["TestProg:counter"]))
        manager.register_data_callback(lambda event: data.append(event.payload["data"]))
        try:
            first = system.get_component("PLC1")
            self.assertIsNotNone(first)
            # The mirror stays on: replacing the stage under a mirrored PLC
            # must not raise or crash (fixed in the framework, OU #14).
            await app.next_update_async()
            for rescan in ("refresh", "register", "stage"):
                if rescan == "refresh":
                    system.find_and_create_components()
                elif rescan == "register":
                    registry.register("br", BrDriver, OPTIONS, legacy_namespace="br_bridge", title="B&R (OMJSON)")
                else:
                    await omni.usd.get_context().new_stage_async()
                for _ in range(5):
                    await app.next_update_async()
                runtime = system.get_component("PLC1")
                self.assertIsNotNone(runtime, f"PLC1 gone after {rescan}")
                self.assertIn("TestProg:counter", runtime.read_variables, f"read list lost after {rescan}")
                self.assertEqual(runtime.driver.port, mock.port)
                data.clear()
                for _ in range(200):
                    if data:
                        break
                    await app.next_update_async()
                self.assertTrue(data, f"no data after {rescan}")
            self.assertIsNot(system.get_component("PLC1"), first)
        finally:
            manager.cleanup()
            BrBridge._release()
            system.remove_component("PLC1")
            mock.stop()
            for key, value in old.items():
                if value is None:
                    settings.destroy_item(SETTINGS + key)
                else:
                    settings.set(SETTINGS + key, value)

    async def test_stage_close_with_hung_server_does_not_block(self):
        """
        Closing a stage whose B&R PLC stopped answering (the server accepts
        nothing, the worker is stuck in a read) must not stall the main thread
        for the driver's timeouts.
        """
        import omni.kit.app
        from loupe.simulation.bridge import get_system

        app = omni.kit.app.get_app()
        context = omni.usd.get_context()
        await context.new_stage_async()
        mock = _mock_module().MockOmjson({"TestProg:counter": 7}).start()
        system = get_system()
        try:
            runtime = system.add_component("BR1", {
                "bridge:driver": "br", "br:Host": "127.0.0.1", "br:Port": mock.port,
                "bridge:Enable": True, "bridge:RefreshRate": 20,
                "bridge:Variables": ["TestProg:counter"], "bridge:MirrorToUsd": False})
            for _ in range(300):
                if runtime.plc.latest() is not None:
                    break
                await app.next_update_async()
            self.assertIsNotNone(runtime.plc.latest(), "no data from the mock server")
            # Hang the server's loop: the connection stays open, nothing is read
            # or answered, so the worker blocks in a read and a close handshake
            # gets no reply.
            mock.silent = True
            mock._loop.call_soon_threadsafe(time.sleep, 8)
            time.sleep(0.3)
            # The longest gap between two app updates is how long the main
            # thread was blocked; the stage event handler runs inside one.
            ticks = []
            sub = app.get_update_event_stream().create_subscription_to_pop(
                lambda _e: ticks.append(time.monotonic()), name="br-close-timing")
            start = time.monotonic()
            await context.close_stage_async()
            while system.get_component_names():
                await app.next_update_async()
                self.assertLess(time.monotonic() - start, 30, "the stage close never removed the component")
            for _ in range(3):
                await app.next_update_async()
            sub = None
            total = time.monotonic() - start
            stamps = [start] + ticks
            longest = max(b - a for a, b in zip(stamps, stamps[1:]))
            print(f"stage close with a hung B&R server: {total:.2f}s until the component was gone, "
                  f"longest gap between app updates {longest:.2f}s")
            self.assertLess(longest, 1.0, "closing the stage blocked the main thread")
        finally:
            system.remove_component("BR1")
            mock.stop()
