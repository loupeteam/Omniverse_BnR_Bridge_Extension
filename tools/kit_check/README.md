# Headless Kit check

Runs the extension inside a built Kit app without a window, opens a stage with
`/PLC` prims, and checks: startup, one daemon worker per PLC, the options
setter, data delivery (message bus, `on_sample`, `latest()`), the USD mirror,
write-back through `write:value`, write acknowledgement, and disconnect on
disable. Live mode needs a PLC; `FIXCHECK_MODE=inject` feeds synthetic data.

Copied from the Beckhoff repo's `tools/kit_check` (Phase 2 of its
`docs/IMPLEMENTATION_PLAN.md`) with the extension, attribute and symbol names
swapped. `stages/br_test.usda` carries a `/PLC/PLC1` prim pointing at
`127.0.0.1:8000` and reading the `TestProg` symbols of `test/AS Project`, so
live mode runs against that project in ARsim, or against the mock OMJSON server
from `br_bridge/tests/mock_omjson.py` started on port 8000.

Today it is wired to the Moonlight sandbox (`D:\prj\Sandboxed\Moonlight`, Kit
108): `run.sh` carries those paths (override with `MOONLIGHT`) and
`fixcheck.kit.template` is that app's `.kit` with the extension folder
substituted for `${FIXCHECK_EXTS}`. Phase 1 generalises it.

```bash
FIXCHECK_MODE=inject bash tools/kit_check/run.sh exts out.log   # no PLC needed
bash tools/kit_check/run.sh exts out.log                         # live, PLC on 127.0.0.1:8000
```

Both must end with `OK -- all fix checks passed`. `run.sh` is ignored by the
repo's `.gitignore` pattern for `*.sh`; it is force-added.
