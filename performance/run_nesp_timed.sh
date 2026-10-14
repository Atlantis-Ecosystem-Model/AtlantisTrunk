#!/bin/bash
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Timed NESP one-year Atlantis run ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
# Usage, from anywhere:
#   performance/run_nesp_timed.sh baseline_rep1
#
# Writes outputs and the GNU time report under
# performance/nesp_1y/runs/<name>/
# The model inputs are never modified.

set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <run_name>" >&2
    exit 2
fi

RUN_NAME="$1"
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MODEL_DIR="${REPO_ROOT}/performance/nesp_1y/model"
OUT_DIR="${REPO_ROOT}/performance/nesp_1y/runs/${RUN_NAME}"
# Frozen baseline unless ATLANTIS_EXE names a candidate binary.
EXE="${ATLANTIS_EXE:-${REPO_ROOT}/performance/nesp_1y/bin/atlantisMerged}"
QUIET_FLAG=""
if [ "${ATLANTIS_QUIET:-0}" = "1" ]; then
    QUIET_FLAG="-Q"
fi

if [ ! -x "$EXE" ]; then
    echo "Atlantis executable not found: $EXE" >&2
    exit 1
fi
if [ ! -f "${MODEL_DIR}/NESP_run.prm" ]; then
    echo "Model inputs not found: $MODEL_DIR" >&2
    exit 1
fi
if [ -e "$OUT_DIR" ]; then
    echo "Refusing to overwrite an existing run directory: $OUT_DIR" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"
cd "$MODEL_DIR"

export LD_LIBRARY_PATH="/usr/local/proj4-4.9.3/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

# /usr/bin/time -v writes its report to stderr. Atlantis warnings also go to
# stderr, so the time report is captured separately with -o.
set +e
# Timestamp each stdout line so initialisation can be split from the timestep loop.
/usr/bin/time -v -o "${OUT_DIR}/time.txt" \
    stdbuf -oL -eL "$EXE" \
        -i NESP_init.nc 0 \
        -o output.nc \
        -r NESP_run.prm \
        -m NESP_migration.csv \
        -f NESP_force_noevap.prm \
        -p NESP_physics.prm \
        -b NESP_bio.prm \
        -s NESP_groups.csv \
        -d "$OUT_DIR" \
        ${QUIET_FLAG} \
    > >(gawk '{ print strftime("%Y-%m-%dT%H:%M:%S"), $0; fflush() }' > "${OUT_DIR}/atlantis_stdout.txt") \
    2> "${OUT_DIR}/atlantis_stderr.txt"
RUN_STATUS=$?
set -e
wait || true

echo "exit ${RUN_STATUS}" | tee "${OUT_DIR}/exit_code.txt"
exit "${RUN_STATUS}"
