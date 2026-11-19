#!/bin/bash
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Timed 30-day NESP run of a candidate binary ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
# Usage:
#   performance/run_30d.sh <candidate_name> <run_name>
# Runs performance/nesp_1y/bin/atlantisMerged_<candidate_name> from profile_30d
# (warnings off, -Q), writes outputs to performance/nesp_1y/runs/<run_name>/,
# and prints the wall-clock and user time. OMP_NUM_THREADS is passed through.

set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <candidate_name> <run_name>" >&2
    exit 2
fi

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
EXE="${REPO_ROOT}/performance/nesp_1y/bin/atlantisMerged_$1"
OUT_DIR="${REPO_ROOT}/performance/nesp_1y/runs/$2"

if [ ! -x "$EXE" ]; then
    echo "Candidate binary not found: $EXE" >&2
    exit 1
fi
if [ -e "$OUT_DIR" ]; then
    echo "Refusing to overwrite an existing run directory: $OUT_DIR" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"
cd "${REPO_ROOT}/performance/nesp_1y/profile_30d"
export LD_LIBRARY_PATH="/usr/local/proj4-4.9.3/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

/usr/bin/time -v -o "${OUT_DIR}/time.txt" "$EXE" \
    -i NESP_init.nc 0 -o output.nc -r NESP_run.prm -m NESP_migration.csv \
    -f NESP_force_noevap.prm -p NESP_physics.prm -b NESP_bio.prm -s NESP_groups.csv \
    -d "$OUT_DIR" -Q > "${OUT_DIR}/stdout.txt" 2> "${OUT_DIR}/stderr.txt"

WALL="$(sed -n 's/.*Elapsed (wall clock).*: //p' "${OUT_DIR}/time.txt")"
USER_SECONDS="$(sed -n 's/.*User time (seconds): //p' "${OUT_DIR}/time.txt")"
echo "$2 wall ${WALL} user ${USER_SECONDS}"
