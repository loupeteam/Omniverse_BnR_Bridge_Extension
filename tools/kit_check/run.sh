#!/bin/bash
# usage: run.sh <exts-folder> <out-log>
#
# Environment (all optional):
#   FIXCHECK_STAGE   stage to open            (default: stages/br_test.usda next to this script)
#   FIXCHECK_MODE    "" live, "inject" synthetic data
#   MOONLIGHT        the Moonlight sandbox     (default: D:/prj/Sandboxed/Moonlight)
#   FIXCHECK_KIT     a ready .kit file         (default: generated from fixcheck.kit.template)
HERE="$(cd "$(dirname "$0")" && pwd)"
ML="${MOONLIGHT:-/d/prj/Sandboxed/Moonlight}"
REL=$ML/kit-app-template/_build/windows-x86_64/release
EXTS=$1; OUT=$2
WORK="${FIXCHECK_WORK:-$HERE/.work}"; mkdir -p "$WORK"
export FIXCHECK_STAGE="${FIXCHECK_STAGE:-$HERE/stages/br_test.usda}"
export FIXCHECK_EXTS="$EXTS"
KIT="${FIXCHECK_KIT:-$WORK/fixcheck.kit}"
if [ -z "$FIXCHECK_KIT" ]; then
  sed "s|\${FIXCHECK_EXTS}|$EXTS|" "$HERE/fixcheck.kit.template" > "$KIT"
fi
start=$(date +%s)
timeout -k 5 150 "$REL/kit/kit.exe" "$KIT" \
  --ext-folder "$REL/exts" --ext-folder "$REL/extscache" --ext-folder "$REL/apps" \
  --ext-folder "$EXTS" \
  --no-window --/log/file="$WORK/kit_fixcheck.log" \
  --exec "$HERE/kit_check.py" > "$OUT" 2>&1
rc=$?
end=$(date +%s)
echo "exit code $rc, wall $((end-start))s"
grep -n 'Failed to startup python extension\|posting quit' "$OUT"
sed -n '/fix check --/,/^====*$/p' "$OUT" | tail -40
