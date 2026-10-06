import threading
import time

import pytest

from br_bridge import NOT_IN_RESPONSE, UNDEFINED, BrDriver
from plc_bridge import PlcDriver, PlcRuntime

from mock_omjson import MockOmjson

VARIABLES = {
    "gCounter": 7,
    "TestProg:axis.position": 12.5,
    "TestProg:axis.name": "jaw",
    "TestProg:status": {"ready": True, "code": 3},
    "TestProg:list[2]": 99,
}


@pytest.fixture
def server():
    s = MockOmjson(VARIABLES).start()
    yield s
    s.stop()


@pytest.fixture
def driver(server):
    d = BrDriver("127.0.0.1", server.port, timeout=2.0)
    yield d
    d.close()


def test_driver_implements_the_contract():
    d = BrDriver("127.0.0.1", 1)
    assert isinstance(d, PlcDriver)
    assert BrDriver.symbol_separators == ":."
    assert not d.is_connected()
    d.disconnect()  # safe when never connected
    d.close()


def test_connect_read_and_disconnect(driver):
    driver.connect()
    assert driver.is_connected()
    result = driver.read(["gCounter", "TestProg:axis.position", "TestProg:status"])
    assert result.values == {
        "gCounter": 7,
        "TestProg:axis.position": 12.5,
        "TestProg:status": {"ready": True, "code": 3},
    }
    assert result.errors == {}
    driver.disconnect()
    assert not driver.is_connected()


def test_unknown_and_undefined_symbols_are_errors_not_values(driver):
    driver.connect()
    result = driver.read(["gCounter", "nope", "undefThing"])
    assert result.values == {"gCounter": 7}
    assert result.errors == {"nope": NOT_IN_RESPONSE, "undefThing": UNDEFINED}


def test_write_reports_rejected_symbols(driver, server):
    driver.connect()
    errors = driver.write({"gCounter": 8, "nope": 1, "undefThing": 2})
    assert errors == {"nope": NOT_IN_RESPONSE, "undefThing": UNDEFINED}
    assert server.variables["gCounter"] == 8
    assert driver.read(["gCounter"]).values == {"gCounter": 8}


def test_read_when_not_connected_raises(driver):
    with pytest.raises(ConnectionError):
        driver.read(["gCounter"])


def test_connect_to_nothing_raises_within_the_timeout():
    d = BrDriver("127.0.0.1", 1, timeout=1.0)  # nothing listens on port 1
    started = time.monotonic()
    with pytest.raises(Exception):
        d.connect()
    assert time.monotonic() - started < 3
    assert not d.is_connected()
    d.close()


def test_a_silent_server_times_out_and_marks_the_link_lost():
    server = MockOmjson(VARIABLES, silent=True).start()
    try:
        d = BrDriver("127.0.0.1", server.port, timeout=0.5)
        d.connect()
        started = time.monotonic()
        with pytest.raises(TimeoutError):
            d.read(["gCounter"])
        assert time.monotonic() - started < 1.5
        assert not d.is_connected()
        d.close()
    finally:
        server.stop()


def test_disconnect_from_another_thread_unblocks_a_read_in_flight():
    server = MockOmjson(VARIABLES, silent=True).start()
    try:
        d = BrDriver("127.0.0.1", server.port, timeout=5.0)
        d.connect()
        outcome = {}

        def read():
            try:
                d.read(["gCounter"])
                outcome["result"] = "returned"
            except Exception as e:
                outcome["result"] = type(e).__name__

        t = threading.Thread(target=read)
        t.start()
        time.sleep(0.2)
        started = time.monotonic()
        d.disconnect()
        t.join(3)
        assert not t.is_alive()
        assert time.monotonic() - started < 2
        assert outcome["result"] == "ConnectionError"
        assert not d.is_connected()
        d.close()
    finally:
        server.stop()


def test_server_going_away_is_a_connection_error_and_link_lost(driver, server):
    driver.connect()
    server.stop()
    with pytest.raises((ConnectionError, TimeoutError)):
        driver.read(["gCounter"])
    assert not driver.is_connected()


def test_runtime_drives_the_br_driver_and_nests_on_colon_and_dot(driver):
    plc = PlcRuntime(driver, name="BR", enabled=True)
    plc.set_read_variables(["gCounter", "TestProg:axis.position", "TestProg:list[2]"])
    samples = []
    plc.on_sample(samples.append)
    plc.scan()
    assert samples[0].nested == {
        "gCounter": 7,
        "TestProg": {"axis": {"position": 12.5}, "list": [None, None, 99]},
    }
    handle = plc.queue_write("gCounter", 42)
    plc.scan()
    assert handle.ok
    assert samples[-1].values["gCounter"] == 42
    plc.enabled = False
    plc.scan()  # driven by hand, so disable rather than stop() a thread that was never started
    assert not driver.is_connected()


def test_runtime_polls_in_the_background(driver):
    plc = PlcRuntime(driver, name="BRT", enabled=True, refresh_ms=10)
    plc.set_read_variables(["gCounter"])
    got = threading.Event()
    plc.on_sample(lambda s: got.set() if s.seq >= 5 else None)
    plc.start()
    assert got.wait(3)
    plc.stop()
    assert not driver.is_connected()
