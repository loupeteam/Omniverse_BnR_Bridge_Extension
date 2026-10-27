# br_bridge

The B&R PLC driver for Loupe's Omniverse bridges, as a plain Python package. It
talks OMJSON over a websocket to a PLC running
[jsonWebSocketServer](https://loupeteam.github.io/LoupeDocs/libraries/omjson/jsonwebsocketserver.html).
It imports nothing from Omniverse or Kit; it depends on
[websockets](https://github.com/python-websockets/websockets) and on
`plc_bridge`, the vendor-neutral runtime and driver contract from
[Omni-Utils](https://github.com/loupeteam/Omni-Utils).

`BrDriver` implements the `plc_bridge.PlcDriver` contract, so it is
interchangeable with the Beckhoff `AdsDriver`:

```python
from plc_bridge import PlcRuntime
from br_bridge import BrDriver          # or: from beckhoff_bridge import AdsDriver

# Symbols are illustrative: gCounter exists in test/AS Project, TestProg:axis does not.
plc = PlcRuntime(BrDriver("192.168.0.61", 8000), refresh_ms=20, enabled=True)
plc.set_read_variables(["gCounter", "TestProg:axis.position"])
plc.on_sample(lambda s: print(s.seq, s.nested))   # {"gCounter": 7, "TestProg": {"axis": {"position": 12.5}}}
plc.on_problem(print)
plc.start()
...
handle = plc.queue_write("gCounter", 42)
handle.wait(1.0); print(handle.ok)
...
plc.stop()
```

## How it maps OMJSON onto the contract

| Contract | OMJSON |
|---|---|
| `connect()` | `websockets.connect("ws://host:port")` with the driver's timeout; no pings (the server does not answer them). |
| `read(symbols)` | one `{"type": "read", "data": [...]}`; the `readresponse` list is flattened. A symbol missing from the reply is reported as `not in response`, one returned as `"undefined"` as `undefined`. Both go in `ReadResult.errors`, never as values. |
| `write(values)` | one `{"type": "write", "data": {...}}`; symbols missing or `"undefined"` in the `writeresponse` are returned as rejected. OMJSON 2.0.0 does neither for an unknown symbol: it echoes the request, so the driver also rejects a symbol whose most recent read failed (`<reason> on the last read`). A write to an unknown symbol that is never read is not caught. OMJSON is deprecated upstream and this will not be fixed there, so the gap is permanent: read a symbol before relying on a write to it. A write the server cannot parse gets no reply: the call times out after `timeout` and the link is reported lost, so the runtime reconnects. |
| `disconnect()` | sends an empty frame first (OMJSON does not handle the close opcode well), then closes. Unblocks a read or write in flight on another thread. |
| `is_connected()` | the socket is open and no timeout or closed-connection error has been seen since `connect()`. |
| `symbol_separators` | `":."`, so `TestProg:axis.position` nests as `TestProg / axis / position`. |

The transport is asyncio. The driver owns an event loop on a daemon thread and
exposes the synchronous contract on top of it; every call is bounded by the
driver's `timeout` (3 s by default). One request is in flight at a time, since
OMJSON has no correlation ids. `close()` disconnects and stops the loop thread
when the driver is no longer needed.

## Tests

```bash
pip install -e <path to Omni-Utils>/plc_bridge
pip install -e .[test]
pytest
```

The tests run a mock OMJSON server in-process (`tests/mock_omjson.py`) and do
not need a PLC.
