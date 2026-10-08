"""
Headless Kit check for the B&R extension on the PLC bridge framework.

Run inside Kit with --exec (run.sh / run.ps1 do that). Environment:
  FIXCHECK_STAGE     stage to open; default stages/br_test.usda next to this file: a
                     0.3.0rc1 prim /PLC/PLC1 (br_bridge:*) and a neutral prim /PLC/BR2
                     (bridge:driver = "br")
  FIXCHECK_MODE      "" = live against a mock OMJSON server started here (both prims'
                     ports are pointed at it), "inject" = no server: an in-memory fake
                     driver replaces BrDriver under "br", "arsim" = live against the
                     PLC the stage names (test/AS Project in ARsim, 127.0.0.1:8000)
  FIXCHECK_BR_TESTS  folder holding br_bridge's mock_omjson.py (default: br_bridge/tests
                     in this checkout)
Prints one line per check and "OK -- all fix checks passed" or "FAIL ...".

What a 0.1.x user has is covered twice: a 0.3.0rc1 stage with a script on
`BrBridge.Manager("PLC1")`, and a stage with no PLC prim at all where the
script calls `BrBridge.Manager()` and the PLC comes from the 0.1.0 persistent
settings. Both must work unchanged, with deprecation warnings.
"""

import asyncio
import importlib.util
import logging
import os
import sys
import threading
import time
import warnings

import carb.settings
import omni.kit.app
import omni.usd

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
STAGE = os.path.abspath(os.environ.get("FIXCHECK_STAGE") or os.path.join(HERE, "stages", "br_test.usda")).replace("\\", "/")
MODE = os.environ.get("FIXCHECK_MODE", "")
ARSIM = MODE == "arsim"
EXTS = ("loupe.simulation.br_bridge", "loupe.simulation.bridge")
LIVE_SEC = 5.0
MAIN_THREAD = threading.current_thread()
LEGACY_SETTINGS = "/persistent/loupe.simulation.br_bridge/"
VARIABLES = {
    "TestProg:counter": 7, "TestProg:counter2": 8, "TestProg:lreal": 1.5, "TestProg:bool": True,
    "TestProg:structOfStructs": {"var1": 3, "secondStruct": {"bool": True}},
    "TestProg:structOfStructs.var1": 3, "TestProg:structOfStructs.secondStruct.bool": True,
    "TestProg:arr": [10.0, 11.0, 12.0],
}

fails = []
app = omni.kit.app.get_app()
mgr = app.get_extension_manager()

print("fix check -- Kit {}, Python {}.{}".format(app.get_build_version(), *sys.version_info[:2]))
print("=" * 68)


def _ver(e):
    v = e.get("version", "")
    return ".".join(str(x) for x in v if x != "") if isinstance(v, (tuple, list)) else str(v)


def quit_after(seconds):
    # omni.kit.window.file cancels a headless quit on a dirty stage; do not let
    # that, or a failed import, keep the process alive.
    threading.Thread(target=lambda: (time.sleep(seconds), os._exit(7)), daemon=True).start()


# --- 1. both extensions registered and enabled ------------------------------------
for ext in EXTS:
    known = [e for e in mgr.get_extensions() if e.get("name", "") == ext]
    for e in known:
        print("  registered   {} {}  enabled={}  path={}".format(
            e.get("name", ""), _ver(e), e.get("enabled", False), e.get("path", "")))
    if not any(e.get("enabled", False) for e in known):
        fails.append("{} not enabled".format(ext))


class Capture(logging.Handler):
    def __init__(self):
        super().__init__(logging.WARNING)
        self.messages = []

    def emit(self, record):
        self.messages.append(record.getMessage())


captured = Capture()
logging.getLogger("loupe.simulation").addHandler(captured)

try:
    from br_bridge import BrDriver  # noqa: E402
    from loupe.simulation.bridge import get_system, on_sample_main, registry  # noqa: E402
    from loupe.simulation.bridge.bus import EVENT_TYPE_DATA_READ, get_stream_name  # noqa: E402
    import plc_bridge  # noqa: E402
    from plc_bridge import Sample  # noqa: E402
except Exception as e:
    import traceback
    traceback.print_exc()
    print("FAIL -- import: {!r}".format(e), flush=True)
    app.post_quit()
    quit_after(15)
    raise

print("  startup      clean; plc_bridge from {}; br_bridge from {}".format(
    os.path.dirname(plc_bridge.__file__), os.path.dirname(sys.modules["br_bridge"].__file__)))


class FakeBrDriver(plc_bridge.PlcDriver):
    """Inject mode: answers the stage's symbols from memory, no server."""

    symbol_separators = ":."

    def __init__(self, host="127.0.0.1", port=8000):
        self.host, self.port = host, port
        self.values = dict(VARIABLES)
        self.connected = False

    def connect(self):
        self.connected = True

    def disconnect(self):
        self.connected = False

    def is_connected(self):
        return self.connected

    def read(self, symbols):
        values = {s: self.values[s] for s in symbols if s in self.values}
        errors = {s: "not in response" for s in symbols if s not in self.values}
        return plc_bridge.ReadResult(values, errors)

    def write(self, values):
        self.values.update(values)
        return {}


# --- 2. the driver: registered by the vendor extension, not by this script ---------
spec = registry.get("br")
print("  driver br    class={} legacy_namespace={} options={}".format(
    spec and spec.driver_class.__name__, spec and spec.legacy_namespace,
    spec and [(o.key, o.kind, o.default) for o in spec.options]))
if spec is None or spec.driver_class is not BrDriver or spec.legacy_namespace != "br_bridge":
    fails.append("loupe.simulation.br_bridge did not register BrDriver under 'br'")
if MODE == "inject" and spec is not None:
    registry.register("br", FakeBrDriver, spec.options, legacy_namespace="br_bridge")
    print("  mode         inject: FakeBrDriver registered under 'br', no server")


def load_mock_omjson():
    folder = os.environ.get("FIXCHECK_BR_TESTS") or os.path.join(REPO, "br_bridge", "tests")
    path = os.path.join(folder, "mock_omjson.py")
    module_spec = importlib.util.spec_from_file_location("mock_omjson", path)
    module = importlib.util.module_from_spec(module_spec)
    module_spec.loader.exec_module(module)
    return module


async def ticks(n):
    for _ in range(n):
        await app.next_update_async()


async def wait_for(predicate, seconds):
    t0 = time.time()
    while time.time() - t0 < seconds and not predicate():
        await app.next_update_async()
    return predicate()


async def main():
    ctx = omni.usd.get_context()
    system = get_system()
    settings = carb.settings.get_settings()
    mock = None
    if ARSIM:
        print("  plc          ARsim: the stage's hosts and ports, no mock (test/AS Project must be running)")
    elif MODE != "inject":
        mock = load_mock_omjson().MockOmjson(VARIABLES).start()
        print("  mock omjson  127.0.0.1:{} (no ARsim: live means this mock server)".format(mock.port))

    # --- 3. the stage: one 0.3.0rc1 prim, one neutral prim, both driver "br" ------
    await ctx.open_stage_async(STAGE)
    await ticks(10)
    names = system.get_component_names()
    print("  components   after stage open: {}".format(names))
    if "PLC1" not in names or "BR2" not in names:
        fails.append("expected PLC1 and BR2 runtimes from the stage, got {}".format(names))
        return
    legacy_rt, neutral_rt = system.get_component("PLC1"), system.get_component("BR2")
    for rt in (legacy_rt, neutral_rt):
        print("  {:<12} driver={} legacy={} vars={} driver class={}".format(
            rt.name, rt.driver_name, rt.legacy, len(rt.read_variables), type(rt.driver).__name__))
    if not (legacy_rt.driver_name == "br" and legacy_rt.legacy and neutral_rt.driver_name == "br" and not neutral_rt.legacy):
        fails.append("prim classification wrong")
    prim_warning = [m for m in captured.messages if "/PLC/PLC1" in m and "DEPRECATED" in m]
    print("  prim warning {}".format(prim_warning[0][:100] + "..." if prim_warning else None))
    if not prim_warning:
        fails.append("no deprecation warning for the br_bridge:* prim")
    threads = [t for t in threading.enumerate() if t.name.endswith("-plc")]
    print("  threads      {} worker(s) {}, daemon={}".format(len(threads), sorted(t.name for t in threads), [t.daemon for t in threads]))
    if len(threads) != 2 or not all(t.daemon for t in threads):
        fails.append("expected one daemon worker per PLC")
    if mock is not None:
        legacy_rt.options = {"br_bridge:Port": mock.port}
        neutral_rt.options = {"br:Port": mock.port}
        print("  port         -> {}: PLC1 driver.port={} BR2 driver.port={}".format(
            mock.port, legacy_rt.driver.port, neutral_rt.driver.port))
        if legacy_rt.driver.port != mock.port or neutral_rt.driver.port != mock.port:
            fails.append("option change did not reach the driver")

    # --- 4. a 0.3.0rc1 script, unchanged: BrBridge.Manager("PLC1") ----------------
    sys.modules.pop("loupe.simulation.br_bridge.BrBridge", None)
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        from loupe.simulation.br_bridge import BrBridge
    import_warnings = [str(w.message)[:80] for w in caught if issubclass(w.category, DeprecationWarning)]
    print("  BrBridge     import DeprecationWarning: {}".format(import_warnings[:1]))
    if not import_warnings:
        fails.append("importing BrBridge did not warn")
    script = {"init": 0, "data": 0, "last": None}

    def on_plc_init(event):
        script["init"] += 1
        br.add_cyclic_read_variables(["TestProg:counter2"])

    def on_message(event):
        script["data"] += 1
        script["last"] = event.payload["data"]["TestProg"]["counter2"]

    br = BrBridge.Manager("PLC1")
    br.register_init_callback(on_plc_init)
    br.register_data_callback(on_message)

    # --- 5. data flows on every surface ------------------------------------------
    bus = app.get_message_bus_event_stream()
    counts = {k: 0 for k in ("legacy_bus", "neutral_bus", "br2_legacy_bus", "br2_neutral_bus", "plc1_cb", "br2_cb", "br2_main")}
    main_threads = set()

    def count(key):
        return lambda *_: counts.__setitem__(key, counts[key] + 1)

    subs = [
        bus.create_subscription_to_push_by_type(get_stream_name(BrBridge.EVENT_TYPE_DATA_READ, "PLC1"), count("legacy_bus")),
        bus.create_subscription_to_push_by_type(get_stream_name(EVENT_TYPE_DATA_READ, "PLC1"), count("neutral_bus")),
        bus.create_subscription_to_push_by_type(get_stream_name(BrBridge.EVENT_TYPE_DATA_READ, "BR2"), count("br2_legacy_bus")),
        bus.create_subscription_to_push_by_type(get_stream_name(EVENT_TYPE_DATA_READ, "BR2"), count("br2_neutral_bus")),
    ]
    removers = [
        legacy_rt.plc.on_sample(count("plc1_cb")),
        neutral_rt.plc.on_sample(count("br2_cb")),
        on_sample_main("BR2", lambda s: (counts.__setitem__("br2_main", counts["br2_main"] + 1),
                                         main_threads.add(threading.current_thread() is MAIN_THREAD))),
    ]
    status = []
    neutral_rt.plc.on_connection(status.append)
    legacy_rt.enable_communication = True
    neutral_rt.enable_communication = True
    t0 = time.time()
    while time.time() - t0 < LIVE_SEC:
        await app.next_update_async()
    latest = neutral_rt.plc.latest()
    print("  live {:.0f}s      PLC1: legacy bus={} neutral bus={} on_sample={} script init={} data={} counter2={}".format(
        LIVE_SEC, counts["legacy_bus"], counts["neutral_bus"], counts["plc1_cb"], script["init"], script["data"], script["last"]))
    print("               BR2:  legacy bus={} neutral bus={} on_sample={} on_sample_main={} main_thread_only={} latest.seq={}".format(
        counts["br2_legacy_bus"], counts["br2_neutral_bus"], counts["br2_cb"], counts["br2_main"],
        main_threads == {True}, latest.seq if isinstance(latest, Sample) else None))
    print("  status       BR2 {}".format(status[:4]))
    for key in ("legacy_bus", "neutral_bus", "br2_legacy_bus", "br2_neutral_bus", "br2_main"):
        if counts[key] == 0:
            fails.append("nothing on {}".format(key))
    for key in ("plc1_cb", "br2_cb"):
        if counts[key] < 0.6 * LIVE_SEC * 50:
            fails.append("{} well below 50 Hz: {} in {}s".format(key, counts[key], LIVE_SEC))
    expected = isinstance(script["last"], int) if ARSIM else script["last"] == VARIABLES["TestProg:counter2"]
    if script["data"] == 0 or not expected:
        fails.append("the BrBridge.Manager('PLC1') script got no data for the variable it added")
    if main_threads != {True}:
        fails.append("on_sample_main delivered off the main thread")
    if "Connected" not in status:
        fails.append("BR2 never reported Connected")

    # --- 6. mirror: nested value, array element, the colon kept ---------------------
    stage = ctx.get_stage()
    checks = {
        "PLC1 nested": "/PLC/PLC1/TestProg/structOfStructs/secondStruct/bool",
        "BR2 nested ": "/PLC/BR2/TestProg/structOfStructs/secondStruct/bool",
        "BR2 array  ": "/PLC/BR2/TestProg/arr/_1",
        "BR2 scalar ": "/PLC/BR2/TestProg/lreal",
    }
    for label, path in checks.items():
        prim = stage.GetPrimAtPath(path)
        valid = bool(prim and prim.IsValid())
        value = prim.GetAttribute("value").Get() if valid else None
        symbol = prim.GetAttribute("symbol").Get() if valid else None
        print("  mirror {} valid={} value={} symbol={}".format(label, valid, value, symbol))
        if not valid or value is None:
            fails.append("{} not mirrored at {}".format(label.strip(), path))
    lreal = stage.GetPrimAtPath("/PLC/BR2/TestProg/lreal")
    if lreal and lreal.IsValid() and lreal.GetAttribute("symbol").Get() != "TestProg:lreal":
        fails.append("mirror prim lost the ':' in its symbol")

    # --- 7. writes: write:value edit, Manager.write_variable, queue_write ack -------
    writes = []
    removers.append(neutral_rt.plc.on_write(writes.append))
    before = neutral_rt.plc.latest()
    lreal_before = before.values.get("TestProg:lreal") if ARSIM and isinstance(before, Sample) else None

    def read_back(symbol):
        sample = neutral_rt.plc.latest()
        return sample.values.get(symbol) if isinstance(sample, Sample) else None

    try:
        if ARSIM:
            # TestProg counts every cycle; stop it so every value written reads back exactly.
            stop = neutral_rt.queue_write("TestProg:counterOn", False)
            if not (stop.wait(2.0) and stop.ok):
                fails.append("could not stop the ARsim counters: {}".format(stop.error))
        if lreal and lreal.IsValid():
            attr = lreal.GetAttribute("write:value")
            new_value = (attr.Get() or 0.0) + 1.0
            attr.Set(new_value)
            await wait_for(lambda: any("TestProg:lreal" in w.values for w in writes), 2.0)
            sent = [w for w in writes if "TestProg:lreal" in w.values]
            print("  write-back   BR2 value={} sent={}".format(new_value, sent[-1] if sent else writes))
            if not sent or sent[-1].error or sent[-1].errors:
                fails.append("write:value edit was not written as TestProg:lreal")
            if ARSIM:
                # OMJSON echoes any write, so only a read proves it reached the PLC.
                await wait_for(lambda: read_back("TestProg:lreal") == new_value, 2.0)
                print("               read back {}".format(read_back("TestProg:lreal")))
                if read_back("TestProg:lreal") != new_value:
                    fails.append("TestProg:lreal did not read back as written")
        handle = neutral_rt.queue_write("TestProg:counter", 42)
        got = handle.wait(2.0)
        print("  write ack    done={} ok={} error={}".format(got, handle.ok, handle.error))
        if not (got and handle.ok):
            fails.append("write not acknowledged")
        if ARSIM:
            await wait_for(lambda: read_back("TestProg:counter") == 42, 2.0)
            print("               read back {}".format(read_back("TestProg:counter")))
            if read_back("TestProg:counter") != 42:
                fails.append("TestProg:counter did not read back as written")
        br.write_variable("TestProg:counter2", 99)
        await wait_for(lambda: script["last"] == 99, 2.0)
        print("  Manager      write_variable -> counter2 read back {}".format(script["last"]))
        if script["last"] != 99:
            fails.append("BrBridge.Manager.write_variable did not reach the PLC")
    finally:
        if ARSIM:
            # Put the PLC back even when a check above raised.
            restore = {"TestProg:counterOn": True}
            if lreal_before is not None:
                restore["TestProg:lreal"] = lreal_before
            handles = [neutral_rt.queue_write(name, value) for name, value in restore.items()]
            print("  restore      {} ok={}".format(restore, [h.wait(2.0) and h.ok for h in handles]))
    if mock is not None:
        mock_writes = [r["data"] for r in mock.requests if r.get("type") == "write"]
        print("  mock saw     {} write(s), last {}".format(len(mock_writes), mock_writes[-1:] or None))
        if not any("TestProg:lreal" in w for w in mock_writes):
            fails.append("the mock server never saw the TestProg:lreal write")
    for remove in removers:
        remove()
    subs.clear()
    br.cleanup()

    # --- 8. a 0.1.x stage (no PLC prim) and a 0.1.0 script: BrBridge.Manager() ------
    old = {}
    for key, value in (("PLC_IP_ADDRESS", "127.0.0.1"), ("PLC_PORT", mock.port if mock else 8000),
                       ("REFRESH_RATE", 20), ("ENABLE_COMMUNICATION", True)):
        old[key] = settings.get(LEGACY_SETTINGS + key)
        settings.set(LEGACY_SETTINGS + key, value)
    await ctx.new_stage_async()
    await ticks(5)
    print("  0.1.x stage  components after new stage: {}".format(system.get_component_names()))
    legacy_data = []
    with warnings.catch_warnings(record=True) as caught:
        warnings.simplefilter("always")
        br0 = BrBridge.Manager()
    br0.register_init_callback(lambda event: br0.add_cyclic_read_variables(["TestProg:counter"]))
    br0.register_data_callback(lambda event: legacy_data.append(event.payload["data"]))
    await wait_for(lambda: len(legacy_data) >= 5, 4.0)
    rt0 = system.get_component("PLC1")
    # The mirror puts /PLC/PLC1/TestProg/... in the session layer, so the prim
    # exists on the stage; what must not happen is a spec in the user's layer.
    authored = ctx.get_stage().GetRootLayer().GetPrimAtPath("/PLC/PLC1") is not None
    warned = [str(w.message)[:70] for w in caught if issubclass(w.category, DeprecationWarning)]
    print("  Manager()    warning={}".format(warned[:1]))
    print("               PLC1 driver={} port={} enabled={} data={} first={} prim authored={}".format(
        rt0 and rt0.driver_name, rt0 and rt0.driver.port, rt0 and rt0.enable_communication,
        len(legacy_data), legacy_data[0] if legacy_data else None, authored))
    if not warned:
        fails.append("Manager() without a name did not warn")
    if rt0 is None or rt0.driver_name != "br":
        fails.append("Manager() did not create PLC1 from the 0.1.0 settings")
    if not legacy_data:
        fails.append("the 0.1.0 script got no data")
    if authored:
        fails.append("the in-memory PLC1 was authored into the stage")
    br0.cleanup()
    for key, value in old.items():
        if value is None:
            settings.destroy_item(LEGACY_SETTINGS + key)
        else:
            settings.set(LEGACY_SETTINGS + key, value)

    # --- 9. disable disconnects ---------------------------------------------------------
    if rt0 is not None:
        rt0.enable_communication = False
        await ticks(30)
        print("  connected    PLC1={} after disable".format(rt0.is_connected))
        if rt0.is_connected:
            fails.append("still connected after disable")
    if mock is not None:
        mock.stop()

    print("-" * 68)
    if fails:
        print("FAIL -- " + "; ".join(fails))
    else:
        print("OK -- all fix checks passed")
    print("=" * 68)


async def run():
    try:
        await main()
    except Exception as e:
        import traceback
        traceback.print_exc()
        print("FAIL -- exception: {!r}".format(e))
    sys.stdout.flush()
    print("posting quit at {}".format(time.time()), flush=True)
    app.post_quit()
    quit_after(15)


asyncio.ensure_future(run())
