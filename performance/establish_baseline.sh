#!/bin/bash
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Archive the live NESP run, then time three repeats ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
# Does not start a second Atlantis process while the live run is still going.
# The live process is never signalled.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIVE_PID="${1:-60086}"
LIVE_OUT="/home/por07g/Documents/Projects/NESP_ParksAustralia/Atlantis/whacamole_local/results/runs/local_1y/runs/0000_NESP_bio/outputs"
BASELINE_DIR="${REPO_ROOT}/performance/nesp_1y/baselines/nesp_1y_binary_8fc1952e"
RUN_SCRIPT="${REPO_ROOT}/performance/run_nesp_timed.sh"
COMPARE="${REPO_ROOT}/performance/compare_atlantis_outputs.py"
LOG="${REPO_ROOT}/performance/nesp_1y/runs/baseline_pipeline.log"

mkdir -p "${REPO_ROOT}/performance/nesp_1y/runs"
exec > >(tee -a "$LOG") 2>&1

echo "BASELINE_PIPELINE_START $(date -Is)"

if [ ! -d "$BASELINE_DIR" ]; then
    if kill -0 "$LIVE_PID" 2>/dev/null; then
        echo "Waiting for live Atlantis pid ${LIVE_PID} to exit. It will not be interrupted."
        while kill -0 "$LIVE_PID" 2>/dev/null; do
            sleep 30
        done
        echo "Live pid ${LIVE_PID} has exited $(date -Is)"
        # Give the parent script a moment to flush logs.
        sleep 10
    else
        echo "Live pid ${LIVE_PID} is not running. Archiving outputs that are already on disk."
    fi

    if [ ! -f "${LIVE_OUT}/output.nc" ]; then
        echo "BASELINE_PIPELINE_FAILED missing ${LIVE_OUT}/output.nc"
        exit 1
    fi

    mkdir -p "$BASELINE_DIR"
    rsync -a "$LIVE_OUT/" "$BASELINE_DIR/outputs/"
    python3 - << PY
import netCDF4
path = "${BASELINE_DIR}/outputs/output.nc"
nc = netCDF4.Dataset(path, "r")
print("archived_output_t", nc.dimensions["t"].size)
nc.close()
PY
    {
        echo "Atlantis baseline identity"
        echo "archived_at: $(date -Is)"
        echo "git_commit: $(git -C "$REPO_ROOT" rev-parse HEAD)"
        echo "binary: ${REPO_ROOT}/performance/nesp_1y/bin/atlantisMerged"
        echo "binary_sha256: $(sha256sum "${REPO_ROOT}/performance/nesp_1y/bin/atlantisMerged")"
        echo "compiler_flags_from_dwarf: -g -O2 -std=c99 -mtune=generic -march=x86-64"
        echo "live_source_outputs: ${LIVE_OUT}"
        echo "command: atlantisMerged -i NESP_init.nc 0 -o output.nc -r NESP_run.prm -m NESP_migration.csv -f NESP_force_noevap.prm -p NESP_physics.prm -b NESP_bio.prm -s NESP_groups.csv -d outputs"
        echo "cadence: dt 12 hour, tstop 365 day, toutinc 7 day"
        echo "fisheries: off"
        echo "economics: off"
        echo "LD_LIBRARY_PATH: /usr/local/proj4-4.9.3/lib"
        if [ -f /home/por07g/Documents/Projects/NESP_ParksAustralia/Atlantis/whacamole_local/results/local_1y_batch.log ]; then
            echo "live_batch_log_tail:"
            tail -n 20 /home/por07g/Documents/Projects/NESP_ParksAustralia/Atlantis/whacamole_local/results/local_1y_batch.log
        fi
    } > "${BASELINE_DIR}/BASELINE_INFO.txt"
    echo "ARCHIVE_DONE $(date -Is)"
else
    echo "Baseline archive already exists: $BASELINE_DIR"
    echo "ARCHIVE_DONE $(date -Is)"
fi

echo "Self-comparing the archived outputs"
python3 "$COMPARE" "${BASELINE_DIR}/outputs" "${BASELINE_DIR}/outputs" \
    --report "${BASELINE_DIR}/self_compare.txt"
echo "SELF_COMPARE_DONE $(date -Is)"

for run_name in baseline_rep1 baseline_rep2 baseline_rep3; do
    echo "START ${run_name} $(date -Is)"
    set +e
    "$RUN_SCRIPT" "$run_name"
    run_status=$?
    set -e
    echo "FINISH ${run_name} exit ${run_status} $(date -Is)"
    python3 "$COMPARE" \
        "${BASELINE_DIR}/outputs" \
        "${REPO_ROOT}/performance/nesp_1y/runs/${run_name}" \
        --report "${REPO_ROOT}/performance/nesp_1y/runs/${run_name}/compare_to_baseline.txt" \
        || echo "COMPARE_DIFFERS ${run_name}"
done

echo "BASELINE_PIPELINE_DONE $(date -Is)"
