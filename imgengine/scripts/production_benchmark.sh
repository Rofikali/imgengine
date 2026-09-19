#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${root_dir}/build/benchmark"
build_dir_explicit=0
sample="${root_dir}/photo.jpg"
iterations=1000
warmup=100
preset="passport-45x35"
results_dir=""
portable_baseline=0
build_profile="optimized"

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --build-dir <path>    CMake build directory
  --sample <path>       JPEG/PNG fixture
  --iterations <n>      Measured iterations (default: ${iterations})
  --warmup <n>          Warm-up iterations (default: ${warmup})
  --preset <name>       Native layout template (default: ${preset})
  --results-dir <path>  Evidence directory
  --portable-baseline   Build and measure the portable scalar profile
  --help                Show this help
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) build_dir="$2"; build_dir_explicit=1; shift 2 ;;
        --sample) sample="$2"; shift 2 ;;
        --iterations) iterations="$2"; shift 2 ;;
        --warmup) warmup="$2"; shift 2 ;;
        --preset) preset="$2"; shift 2 ;;
        --results-dir) results_dir="$2"; shift 2 ;;
        --portable-baseline) portable_baseline=1; build_profile="portable-scalar"; shift ;;
        --help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage >&2; exit 64 ;;
    esac
done

[[ -f "$sample" ]] || { echo "Fixture not found: $sample" >&2; exit 66; }
[[ "$iterations" =~ ^[1-9][0-9]*$ ]] || { echo "--iterations must be positive" >&2; exit 64; }
[[ "$warmup" =~ ^[0-9]+$ ]] || { echo "--warmup must be non-negative" >&2; exit 64; }

if [[ "$portable_baseline" -eq 1 && "$build_dir_explicit" -eq 0 ]]; then
    build_dir="${root_dir}/build/benchmark-portable"
fi

if [[ -z "$results_dir" ]]; then
    results_dir="${root_dir}/build/benchmark-results/$(date -u +%Y%m%dT%H%M%SZ)"
fi
mkdir -p "$results_dir"

cmake -S "$root_dir" -B "$build_dir" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DIMGENGINE_LTO=OFF -DIMGENGINE_BENCH=ON \
  -DIMGENGINE_PORTABLE_BASELINE="$portable_baseline" \
  -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
cmake --build "$build_dir" --target bench_lat decoder_bench --parallel

{
    echo "timestamp_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "git_revision=$(git -C "$root_dir" rev-parse HEAD 2>/dev/null || echo unknown)"
    if [[ -z "$(git -C "$root_dir" status --porcelain)" ]]; then
        echo "git_worktree=clean"
    else
        echo "git_worktree=dirty"
    fi
    echo "fixture_sha256=$(sha256sum "$sample" | awk '{print $1}')"
    echo "fixture_bytes=$(wc -c < "$sample" | tr -d ' ')"
    echo "preset=$preset"
    echo "iterations=$iterations"
    echo "warmup=$warmup"
    echo "build_profile=$build_profile"
    uname -a
    command -v lscpu >/dev/null && lscpu
    for governor_file in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
        [[ -r "$governor_file" ]] && echo "$(basename "$(dirname "$governor_file")")_governor=$(<"$governor_file")"
    done
    [[ -r /sys/devices/system/cpu/intel_pstate/no_turbo ]] && echo "intel_turbo_disabled=$(</sys/devices/system/cpu/intel_pstate/no_turbo)"
    [[ -r /sys/devices/system/cpu/cpufreq/boost ]] && echo "cpu_boost_disabled=$(</sys/devices/system/cpu/cpufreq/boost)"
    echo "compiler=$(cc --version | head -n 1)"
    echo "cmake_version=$(cmake --version | head -n 1)"
} > "$results_dir/environment.txt"

"$build_dir/bench_lat" --preset "$preset" --iterations "$iterations" --warmup "$warmup" \
  "$sample" | tee "$results_dir/latency.txt"
"$build_dir/decoder_bench" "$sample" auto "$iterations" | tee "$results_dir/decoder.txt"

echo "Benchmark evidence written to: $results_dir"
