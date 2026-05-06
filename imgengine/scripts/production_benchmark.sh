#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
ENGINE_SAMPLE="${ROOT_DIR}/photo.jpg"
CLI_RUNS=100
CLI_WORKERS=""
PRESET=""
RUN_RAW_CLI=0

detect_workers() {
    local detected=""

    if command -v nproc >/dev/null 2>&1; then
        detected="$(nproc)"
    elif command -v getconf >/dev/null 2>&1; then
        detected="$(getconf _NPROCESSORS_ONLN || true)"
    fi

    if [[ -z "${detected}" || "${detected}" -lt 1 ]]; then
        detected=1
    fi

    printf '%s\n' "${detected}"
}

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --build-dir <path>     CMake build directory (default: ${BUILD_DIR})
  --sample <path>        Encoded image for bench_lat (default: ${ENGINE_SAMPLE})
  --cli-runs <n>         Runs for imgengine_cli_bench (default: ${CLI_RUNS})
  --cli-workers <n>      Worker threads for imgengine_cli_bench (default: auto-detected)
  --raw-cli              Also benchmark the CLI raw-rgb24 decode-bypass path
  --preset <name>        Optional bench_lat preset name
  --help                 Show help

This runs:
  1. bench_lat            -> engine hot/cold path truth
  2. imgengine_cli_bench  -> process-per-job CLI overhead
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --sample)
            ENGINE_SAMPLE="$2"
            shift 2
            ;;
        --cli-runs)
            CLI_RUNS="$2"
            shift 2
            ;;
        --cli-workers)
            CLI_WORKERS="$2"
            shift 2
            ;;
        --raw-cli)
            RUN_RAW_CLI=1
            shift 1
            ;;
        --preset)
            PRESET="$2"
            shift 2
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

if [[ ! -f "${ENGINE_SAMPLE}" ]]; then
    echo "Sample image not found: ${ENGINE_SAMPLE}" >&2
    exit 1
fi

if [[ -z "${CLI_WORKERS}" ]]; then
    CLI_WORKERS="$(detect_workers)"
fi

cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target bench_lat imgengine_cli_bench

echo
echo "=== 1. ENGINE TRUTH: bench_lat ==="
if [[ -n "${PRESET}" ]]; then
    "${BUILD_DIR}/bench_lat" --preset "${PRESET}" "${ENGINE_SAMPLE}"
else
    "${BUILD_DIR}/bench_lat" "${ENGINE_SAMPLE}"
fi

echo
echo "=== 2. CLI PROCESS OVERHEAD: imgengine_cli_bench ==="
echo "Using auto-detected CLI workers: ${CLI_WORKERS}"
"${BUILD_DIR}/imgengine_cli_bench" --runs "${CLI_RUNS}" --workers "${CLI_WORKERS}"

if [[ "${RUN_RAW_CLI}" -eq 1 ]]; then
    echo
    echo "=== 3. CLI RAW-RGB24 PATH: imgengine_cli_bench ==="
    "${BUILD_DIR}/imgengine_cli_bench" --runs "${CLI_RUNS}" --workers "${CLI_WORKERS}" --prepare-raw-rgb24
fi

echo
echo "Next step:"
echo "  Benchmark the long-lived service path with:"
echo "  cd ${ROOT_DIR}/../imgengine-saas/api-service"
echo "  API_GENERATE_RATE_LIMIT=100000/minute uv run python ../data/testing_scripts/service_benchmark.py --requests 20"
