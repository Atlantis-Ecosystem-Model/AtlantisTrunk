#!/bin/bash
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Build a candidate without replacing the shared Atlantis ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
# Other tools run:
#   atlantis/atlantismain/atlantisMerged
# and read the NESP files outside this performance directory.
#
# This script compiles in the source tree, saves the new executable under
# performance/nesp_1y/bin/, then puts the frozen baseline binary back in the
# shared location. It refuses to run while any atlantisMerged process is
# using the shared path.
#
# Usage:
#   performance/build_candidate.sh <candidate_name>
#
# Optional settings (environment variables):
#   BUILD_OPT="-O2 -mtune=generic -fopenmp"  replaces OPT from PreRules.am
#   CLEAN_OBJECTS=1  delete object files and libraries first (needed when
#                    BUILD_OPT changes). The shared executable is left alone
#                    until the final link, so it is never missing for long.
#                    Profile files (*.gcda) are kept, so a -fprofile-use build
#                    can read what a -fprofile-generate run wrote.
#   BUILD_MAKE_ARGS="AR=gcc-ar RANLIB=gcc-ranlib"  extra make variables
#                    (link-time optimisation needs the gcc-ar archiver)

set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <candidate_name>" >&2
    exit 2
fi

CANDIDATE_NAME="$1"
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SHARED="${REPO_ROOT}/atlantis/atlantismain/atlantisMerged"
FROZEN="${REPO_ROOT}/performance/nesp_1y/bin/atlantisMerged"
DEST="${REPO_ROOT}/performance/nesp_1y/bin/atlantisMerged_${CANDIDATE_NAME}"

if [ ! -f "$FROZEN" ]; then
    echo "Frozen baseline binary is missing: $FROZEN" >&2
    echo "Refusing to build, because the shared executable could not be restored." >&2
    exit 1
fi

if [ -e "$DEST" ]; then
    echo "Candidate binary already exists: $DEST" >&2
    exit 1
fi

# A running model has the shared executable mapped. Do not replace that file.
if pgrep -af -- "${SHARED}" | grep -v "build_candidate.sh" >/dev/null; then
    echo "A process is still using the shared executable:" >&2
    pgrep -af -- "${SHARED}" | grep -v "build_candidate.sh" >&2 || true
    echo "Refusing to rebuild over it." >&2
    exit 1
fi

FROZEN_SHA="$(sha256sum "$FROZEN" | awk '{print $1}')"

# Whatever happens below, put the frozen baseline back in the shared path.
restore_shared() {
    cp -a "$FROZEN" "$SHARED"
}
trap restore_shared EXIT

cd "${REPO_ROOT}/atlantis"

if [ "${CLEAN_OBJECTS:-0}" = "1" ]; then
    find . -name '*.o' -delete
    find . -name 'lib*.a' -delete
fi

# Extra make variables, split on spaces on purpose (e.g. "AR=gcc-ar RANLIB=gcc-ranlib")
read -r -a EXTRA_MAKE_ARGS <<< "${BUILD_MAKE_ARGS:-}"

if [ -n "${BUILD_OPT:-}" ]; then
    make -j4 OPT="${BUILD_OPT}" "${EXTRA_MAKE_ARGS[@]}"
else
    make -j4 "${EXTRA_MAKE_ARGS[@]}"
fi

if [ ! -x "$SHARED" ]; then
    echo "Build did not produce $SHARED" >&2
    exit 1
fi

cp -a "$SHARED" "$DEST"
# Replace the path other tools use with the untouched baseline binary.
cp -a "$FROZEN" "$SHARED"
RESTORED_SHA="$(sha256sum "$SHARED" | awk '{print $1}')"

if [ "$RESTORED_SHA" != "$FROZEN_SHA" ]; then
    echo "Shared executable was not restored to the frozen baseline." >&2
    echo "frozen:  $FROZEN_SHA" >&2
    echo "shared:  $RESTORED_SHA" >&2
    exit 1
fi

echo "Candidate saved: $DEST"
echo "Shared executable restored: $SHARED"
echo "sha256 $RESTORED_SHA"
