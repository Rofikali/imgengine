#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_DIR}/build"

usage() {
    cat <<'EOF'
Usage:
  scripts/perf.sh stat bench_lat [bench args...]
  scripts/perf.sh record bench_lat [bench args...]
  scripts/perf.sh stat imgengine_cli [cli args...]
  scripts/perf.sh record imgengine_cli [cli args...]
  scripts/perf.sh report bench_lat
  scripts/perf.sh report imgengine_cli

Examples:
  scripts/perf.sh stat bench_lat ../photo.jpg
  scripts/perf.sh stat bench_lat --input-format raw-rgb24 --input-width 4096 --input-height 4096 /tmp/frame.rgb
  scripts/perf.sh record imgengine_cli --input ../photo.jpg --output /tmp/out.jpg --preset passport-45x35 --quiet
EOF
}

if [[ $# -lt 2 ]]; then
    usage
    exit 1
fi

MODE="$1"
TARGET="$2"
shift 2

BIN="${BUILD_DIR}/${TARGET}"
DATA_FILE="${BUILD_DIR}/perf-${TARGET}.data"
STAT_EVENTS="${PERF_STAT_EVENTS:-task-clock,cpu-clock,context-switches,cpu-migrations,page-faults}"
RECORD_EVENT="${PERF_RECORD_EVENT:-cpu-clock}"

if [[ ! -x "${BIN}" ]]; then
    echo "perf.sh: executable not found: ${BIN}" >&2
    echo "Build first with: cmake .. && make -j" >&2
    exit 1
fi

if ! command -v perf >/dev/null 2>&1; then
    echo "perf.sh: 'perf' is not installed or not on PATH" >&2
    exit 1
fi

case "${MODE}" in
    stat)
        exec perf stat -e "${STAT_EVENTS}" "${BIN}" "$@"
        ;;
    record)
        exec perf record -g -e "${RECORD_EVENT}" -o "${DATA_FILE}" "${BIN}" "$@"
        ;;
    report)
        if [[ ! -f "${DATA_FILE}" ]]; then
            echo "perf.sh: no perf data file found at ${DATA_FILE}" >&2
            exit 1
        fi
        exec perf report -i "${DATA_FILE}"
        ;;
    *)
        usage
        exit 1
        ;;
esac
