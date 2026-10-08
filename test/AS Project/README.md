# B&R test project (Automation Studio 6)

A minimal Automation Studio 6 project for running the B&R bridge against a PLC
in ARsim, the Automation Runtime simulator. It serves PLC variables over OMJSON
on `ws://127.0.0.1:8000`, which is what `br_bridge.BrDriver`, the extension and
`tools/kit_check` (`-Mode arsim`) connect to.

It replaces the Automation Studio 4.10 project that used to live here. That
project is gone; to run the bridge against AS 4, take `test/AS Project` from
tag `v0.1.0` or from `main` before 0.3.0 is merged.

## What is in it

| Item | |
|---|---|
| Configuration | `Simulation`: a 5APC4100.TGL1-000 with `Simulation = 1`, Automation Runtime 6.7.6 |
| `TestProg` (Cyclic#3, 20 ms) | the variables the bridge's tests, docs and stages read and write (below) |
| `LuxProg` (Cyclic#8, 2 ms) | `jsonWebSocketServer` on `127.0.0.1:8000`, 2 clients, 200 kB buffers |
| `Logical/Libraries/Loupe` | OMJSON and its dependencies, as C sources |
| `Logical/Libraries/_AS` | the B&R libraries those need: `AsBrStr`, `AsBrWStr`, `AsHttp`, `AsTCP`, `astime`, `operator`, `runtime`, `standard`, `sys_lib` |
| OPC UA server (OpcUaCs 6.6.1) | default config, only so the simulator's deploy tooling can confirm which instance it reached |

Variables (OMJSON spelling):

| Symbol | Type | Notes |
|---|---|---|
| `TestProg:counter`, `TestProg:counter2` | DINT | +1 and +2 every TestProg cycle while `counterOn` |
| `gCounter` | DINT (global) | +1 every TestProg cycle while `counterOn` |
| `TestProg:counterOn` | BOOL | counters run while TRUE (initially TRUE) |
| `TestProg:counterToggle` | BOOL | write TRUE to flip `counterOn`; reset by TestProg |
| `TestProg:bool`, `TestProg:lreal`, `TestProg:real`, `TestProg:dint`, `TestProg:string` | scalars | `lreal` and `real` start at 1.23456 |
| `TestProg:structOfStructs` | `myStruct` | `var1`, `var2`, `secondStruct.bool`, `secondStruct.array[0..5]` |
| `TestProg:bigLrealArray` | ARRAY[0..99] OF LREAL | |
| `TestProg:arr` | ARRAY[0..2] OF LREAL | `[10, 11, 12]`, for the array mirror checks |

`TestProg:int` is declared but OMJSON answers `undefined` for it: `int` is a C
keyword, so the C task cannot create it. It was the same in the AS 4 project.

`TestProg` behaves as it did in the AS 4 project; `gCounter` and `arr` are new.
`LuxProg` is the program package from OMJSON 2.0.0 with the IP and port set
explicitly and the client-disconnect flag removed (it needed the Hammers
library, which the bridge has no use for).

## Library versions

All are Loupe's public repositories, copied from the `src/Ar/<Library>` folder
at the tag named. Each library's `CHANGELOG.md` and `package.json` came with it.

| Library | Version | Source |
|---|---|---|
| OMJSON | 2.0.0 | [loupeteam/OMJSON](https://github.com/loupeteam/OMJSON) tag `v2.0.0` (also `LuxProg`, from `src/Ar/Lux`) |
| VarTools | 1.0.0 | [loupeteam/VarTools](https://github.com/loupeteam/VarTools) tag `v1.0.0` |
| WebSocket | 1.0.0 | [loupeteam/WebSocket](https://github.com/loupeteam/WebSocket) tag `v1.0.0` |
| TCPComm | 1.0.0 | [loupeteam/TCPComm](https://github.com/loupeteam/TCPComm) tag `v1.0.0` |
| StringExt | 1.1.1 | [loupeteam/StringExt](https://github.com/loupeteam/StringExt) tag `v1.1.1` |
| DataBuffer | 1.0.0 | [loupeteam/DataBuffer](https://github.com/loupeteam/DataBuffer) tag `v1.0.0` |

The project skeleton (`AsProject.apj`, `Physical/`, `Logical/Libraries/_AS`)
is OMJSON 2.0.0's `example/AsProject` Intel configuration, renamed
`Simulation`, with its AR version raised from 6.6.2 to 6.7.6 (the AS 6.7
install has the 5APC4100 system files only for 6.7.6).

The project is not managed by the Loupe package manager. To update a library,
copy its `src/Ar/<Library>` folder from a newer tag and rebuild.

## Building and running in ARsim

Tested with Automation Studio 6.7.0.187 (the `.apj` is in AS 6.5 format, which
AS 6.5 and later open), PVI 6.7.0, Automation Runtime 6.7.6 in ARsim.

Automation Studio does not handle long paths, so build through a short
junction when the checkout is deep (a `.claude\worktrees\...` checkout is):

```powershell
New-Item -ItemType Junction -Path C:\a\as\brtest -Target '<repo>\test\AS Project'
& 'C:\Program Files (x86)\BRAutomation\AS6\bin-en\BR.AS.Build.exe' C:\a\as\brtest\AsProject.apj `
    -c Simulation -buildMode Build -simulation -buildRUCPackage
```

A clean build ends with 0 errors and about 220 warnings: nearly all in the
library sources (most from StringExt's gdtoa), plus `TestProg`'s `int` and the
Technology Guarding notice (447). Check
that `Binaries\Simulation\5APC4100_TGL1_000\RUCPackage\RUCPackage.zip` is newer
than the build start: a locked zip is only a warning.

Then install the RUC package into an ARsim folder and start it, from
Automation Studio (Online > Simulation) or with the Runtime Utility Center.
Start it on 127.0.0.1: `LuxProg` binds the server to that address. When the
simulator reaches RUN, `ws://127.0.0.1:8000` answers OMJSON:

```python
from br_bridge import BrDriver
d = BrDriver("127.0.0.1", 8000)
d.connect()
print(d.read(["TestProg:counter", "gCounter", "TestProg:structOfStructs"]).values)
```

The headless Kit check against it:

```powershell
tools\kit_check\run.ps1 -Kit <kit build root> -BridgeExts <Omni-Utils>\exts -Mode arsim
```

Build output (`Temp`, `Binaries`, `Diagnosis`) and AS user files are ignored by
`.gitignore`; only sources are committed.

## Licensing

The project and the Loupe libraries are licensed under the [MIT License](LICENSE).
