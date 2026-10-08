"""Build the wheels the B&R extension bundles until the packages are on PyPI.

The extension lists `websockets` and `br-bridge` as pip requirements and points
pipapi's `archiveDirs` at `exts/loupe.simulation.br_bridge/wheels/`. This
script fills that folder:

  br_bridge   br_bridge/ at this repo's root
  plc_bridge  plc_bridge/ of an Omni-Utils checkout (--plc-bridge)

The archive has to hold everything the install needs: pipapi runs pip with
`--target`, which ignores packages already installed (plc-bridge, installed by
the framework extension, among them), and `--no-index`, so a dependency
missing from the folder fails the whole install. br_bridge is therefore built
with its dependencies resolved, which puts the plc_bridge wheel built first and
websockets into the folder as well.

The websockets wheel pip picks there is built for the interpreter that runs
this script (cp312-win_amd64, say), so the pure-Python websockets wheels are
downloaded next to it: the same version for any platform, and the newest
release that still supports Python 3.10 (Kit 105 to 108). pip then takes the
speedups where they match and the pure wheel everywhere else.

Run it before packaging the extension for a registry, and again after a
version bump. Wheels of the two Loupe packages already in the folder are
removed first, so a stale version cannot shadow the new one.

Usage:
    python tools/build_wheels.py [--plc-bridge DIR] [--out DIR] [--python EXE]

`--plc-bridge` defaults to the environment variable PLC_BRIDGE_SRC, then to
`../Omni-Utils/plc_bridge` next to this repo.
"""

import argparse
import glob
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EXT = os.path.join(ROOT, "exts", "loupe.simulation.br_bridge")
BR_BRIDGE = os.path.join(ROOT, "br_bridge")
DEFAULT_PLC_BRIDGE = os.environ.get("PLC_BRIDGE_SRC") or os.path.join(
    os.path.dirname(ROOT), "Omni-Utils", "plc_bridge")


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--plc-bridge", default=DEFAULT_PLC_BRIDGE, help="plc_bridge/ of an Omni-Utils checkout")
    ap.add_argument("--out", default=os.path.join(EXT, "wheels"),
                    help="archive folder (default: the extension's wheels/)")
    ap.add_argument("--python", default=sys.executable, help="interpreter whose pip builds the wheels")
    args = ap.parse_args(argv)

    sources = {"plc_bridge": os.path.abspath(args.plc_bridge), "br_bridge": BR_BRIDGE}
    for name, src in sources.items():
        if not os.path.isfile(os.path.join(src, "pyproject.toml")):
            sys.exit("{}: no pyproject.toml at {}{}".format(
                name, src, " (pass --plc-bridge <Omni-Utils>/plc_bridge)" if name == "plc_bridge" else ""))
    os.makedirs(args.out, exist_ok=True)

    for name in sources:
        for old in glob.glob(os.path.join(args.out, name + "-*.whl")):
            os.remove(old)
            print("removed  {}".format(os.path.basename(old)))

    # plc_bridge has no dependencies. br_bridge is resolved against the folder
    # (so it takes the plc_bridge wheel just built) and the index for the rest,
    # which lands websockets in the folder as well.
    for name, deps in (("plc_bridge", False), ("br_bridge", True)):
        print("building {} from {}{}".format(name, sources[name], " with dependencies" if deps else ""))
        cmd = [args.python, "-m", "pip", "wheel", "--wheel-dir", args.out]
        cmd += ["--find-links", args.out] if deps else ["--no-deps"]
        subprocess.run(cmd + [sources[name]], check=True, stdout=subprocess.DEVNULL)

    # Pure-Python websockets: the version just resolved, for any platform, and
    # the newest one that installs on Python 3.10.
    pyver = subprocess.run([args.python, "-c", "import sys; print('%d.%d' % sys.version_info[:2])"],
                           check=True, capture_output=True, text=True).stdout.strip()
    resolved = sorted({os.path.basename(w).split("-")[1] for w in glob.glob(os.path.join(args.out, "websockets-*.whl"))})
    for requirement, python_version in [("websockets==" + v, pyver) for v in resolved] + [("websockets>=12", "3.10")]:
        print("fetching pure-Python {} for Python {}".format(requirement, python_version))
        subprocess.run([args.python, "-m", "pip", "download", requirement, "--only-binary=:all:", "--platform", "any",
                        "--python-version", python_version, "--no-deps", "--dest", args.out],
                       check=True, stdout=subprocess.DEVNULL)

    print("\n{}:".format(os.path.relpath(args.out, ROOT)))
    for whl in sorted(glob.glob(os.path.join(args.out, "*.whl"))):
        print("  " + os.path.basename(whl))
    return 0


if __name__ == "__main__":
    sys.exit(main())
