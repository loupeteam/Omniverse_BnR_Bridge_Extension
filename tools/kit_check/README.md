# Headless Kit check

Runs the B&R extension and the framework it registers with
(`loupe.simulation.bridge`, from Omni-Utils) inside a built Kit app without a
window, and checks:

- both extensions start, and `loupe.simulation.br_bridge` registered
  `br_bridge.BrDriver` under the driver name `br` with the legacy namespace
  `br_bridge`;
- the stage's two PLC prims come up under one `System`: `/PLC/PLC1` in the
  0.3.0rc1 form (`br_bridge:*` attributes, warned as deprecated) and `/PLC/BR2`
  in the neutral form (`bridge:driver = "br"`, `br:Host`, `br:Port`);
- one daemon worker per PLC, the options setter in both spellings;
- a script written for 0.3.0rc1 runs unchanged: `from loupe.simulation.br_bridge
  import BrBridge` (warns), `BrBridge.Manager("PLC1")` with an init callback
  that adds a variable and a data callback that reads it, `write_variable`;
- data on the legacy and neutral bus names, `on_sample`, `latest()`, and
  `on_sample_main` on the main thread;
- the mirror (nested member, array element, the `:` kept in `symbol`),
  write-back through `write:value`, and the write acknowledgement;
- a 0.1.x setup runs unchanged: a stage with no PLC prim and the 0.1.0
  persistent settings, where `BrBridge.Manager()` (no name, warns) creates
  `PLC1` in memory, data arrives, and nothing is authored into the stage;
- disconnect on disable.

| File | Role |
|---|---|
| `kit_check.py` | the check, run inside Kit with `--exec`; reads `FIXCHECK_STAGE` and `FIXCHECK_MODE` (`inject`, empty for the mock, or `arsim`) |
| `fixcheck.kit.template` | a USD Composer app depending on `loupe.simulation.br_bridge`; `${FIXCHECK_EXTS}`, `${FIXCHECK_BRIDGE_EXTS}` and `${FIXCHECK_KIT_ROOT}` are filled in |
| `run.sh`, `run.ps1` | generate the `.kit` in a temp folder, run `kit.exe` from there, print the check's lines, exit 0 on `OK` |
| `stages/br_test.usda` | `/PLC/PLC1` with `br_bridge:*` attributes, `/PLC/BR2` with `bridge:driver = "br"`, both at `127.0.0.1:8000` |

Copied from Omni-Utils `tools/kit_check` (Phase 3 of the Beckhoff repo's
`docs/IMPLEMENTATION_PLAN.md`) and reduced to the B&R driver.

## Running

You need a kit-app-template build (the folder holding `kit/kit.exe`, usually
`_build/windows-x86_64/release`) whose `extscache` has USD Composer's
extensions, an Omni-Utils checkout for `loupe.simulation.bridge`, and the
wheels this extension installs:

```
python tools/build_wheels.py --plc-bridge <Omni-Utils>/plc_bridge
```

The generated app names `exts/loupe.simulation.br_bridge/wheels/` as an
app-wide pip archive, so the framework, which starts first, installs
`plc-bridge` from it too and the Omni-Utils checkout needs no build step.
A checkout linked into Kit's Python with Omni-Utils `tools/dev_link.py`
takes precedence over the wheels (pipapi's import check passes first).

```powershell
tools\kit_check\run.ps1 -Kit D:\kit-app-template\_build\windows-x86_64\release -BridgeExts D:\Omni-Utils\exts
tools\kit_check\run.ps1 -Kit ... -Mode inject      # no server: a fake driver under "br"
tools\kit_check\run.ps1 -Kit ... -Mode arsim       # test/AS Project running in ARsim
```

```bash
tools/kit_check/run.sh --kit D:/kit-app-template/_build/windows-x86_64/release --bridge-exts D:/Omni-Utils/exts --log live.log
```

Options: `--kit` / `-Kit` (required), `--exts` (default: this repo's `exts/`),
`--bridge-exts` (default: `../Omni-Utils/exts` next to this repo),
`--stage` (default: `stages/br_test.usda`), `--mode inject|live|arsim`, `--log`
(default: `kit_check.log` in the current folder). Each has an environment
variable fallback: `FIXCHECK_KIT_ROOT`, `FIXCHECK_EXTS`,
`FIXCHECK_BRIDGE_EXTS`, `FIXCHECK_STAGE`, `FIXCHECK_MODE`, `FIXCHECK_LOG`.

A run takes about 40 s and ends with `OK -- all fix checks passed` or
`FAIL -- ...`. Kit's exit code is 7 by design: the script quits the app and,
because `omni.kit.window.file` can cancel a headless quit on a dirty stage,
forces the exit after 15 s. The launchers exit 0 on `OK`.

**Live mode runs against the mock OMJSON server** from
`br_bridge/tests/mock_omjson.py` (or `FIXCHECK_BR_TESTS`), started in-process;
both prims' ports are pointed at it through the options setter.

**ARsim mode** (`-Mode arsim`, `--mode arsim`) starts no mock: both prims
connect to the host and port in the stage, `127.0.0.1:8000`, which is
`test/AS Project` running in ARsim (see its README). The checks are the same,
except that the PLC's values are live: `counter2` is any integer rather than the
mock's 8, and before writing `counter2 = 99` the check stops TestProg's counters
(`TestProg:counterOn = FALSE`) so the read-back is exact. At the end it sets
`counterOn` back to TRUE and restores `TestProg:lreal`. `TestProg:counter` is
left at whatever it has counted to from 42.

## Kit tests

The extension's `omni.kit.test` suite (driver registration, the `BrBridge`
compatibility module, `Manager()` without a name) runs with
`tools\kit_test.ps1 -Kit <kit build root> -BridgeExts <Omni-Utils>\exts`.
