#!/usr/bin/env bash
set -euo pipefail

source_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
repository_dir=""
if repository_dir=$(git -C "$source_dir" rev-parse --show-toplevel 2>/dev/null); then
    git_revision=$(git -C "$repository_dir" rev-parse HEAD)
    git_worktree=$(test -z "$(git -C "$repository_dir" status --porcelain)" && echo clean || echo dirty)
else
    git_revision="${IMGENGINE_GIT_REVISION:-unavailable}"
    git_worktree="unavailable"
fi
build_dir="${source_dir}/build/scheduler-characterization"
results_dir="${source_dir}/build/scheduler-characterization-results/$(date -u +%Y%m%dT%H%M%SZ)"
iterations=30
warmup=5
concurrency=4
queue_capacity=4
real_image=""
camera_fixture="$source_dir/tests/fixtures/cc0_camera_landscape.jpg"
camera_fixture_sha256="95b1d6cdc0421f8582e0761f5509fa0f81aaf44b4905e87b651ea2dc6d574d5e"

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Characterizes the sealed C ABI execution path against Rust bounded admission.
All CI-safe fixtures are deterministic ImageMagick-generated images. An optional
local --real-image is measured locally only and never copied into the repository.

Options:
  --build-dir <path>       Release build directory
  --results-dir <path>     Evidence output directory
  --iterations <n>         Measured requests per workload (default: ${iterations})
  --warmup <n>             Sequential warm-up requests (default: ${warmup})
  --concurrency <n>        Concurrent callers (default: ${concurrency})
  --queue-capacity <n>     Rust admission capacity for comparable runs (default: ${queue_capacity})
  --real-image <path>      Optional local JPEG; never retained by this script
  --help                   Show this message
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir) build_dir="$2"; shift 2 ;;
        --results-dir) results_dir="$2"; shift 2 ;;
        --iterations) iterations="$2"; shift 2 ;;
        --warmup) warmup="$2"; shift 2 ;;
        --concurrency) concurrency="$2"; shift 2 ;;
        --queue-capacity) queue_capacity="$2"; shift 2 ;;
        --real-image) real_image="$2"; shift 2 ;;
        --help) usage; exit 0 ;;
        *) echo "Unknown argument: $1" >&2; usage >&2; exit 64 ;;
    esac
done

for value in "$iterations" "$concurrency" "$queue_capacity"; do
    [[ "$value" =~ ^[1-9][0-9]*$ ]] || { echo "positive integer required" >&2; exit 64; }
done
[[ "$warmup" =~ ^[0-9]+$ ]] || { echo "--warmup must be non-negative" >&2; exit 64; }
[[ -z "$real_image" || -f "$real_image" ]] || { echo "--real-image not found: $real_image" >&2; exit 66; }
[[ -f "$camera_fixture" ]] || { echo "missing project-owned camera fixture: $camera_fixture" >&2; exit 66; }
[[ "$(sha256sum "$camera_fixture" | awk '{print $1}')" == "$camera_fixture_sha256" ]] || {
    echo "camera fixture checksum mismatch" >&2
    exit 65
}
command -v convert >/dev/null || { echo "ImageMagick convert is required" >&2; exit 69; }
command -v python3 >/dev/null || { echo "python3 is required" >&2; exit 69; }

fixtures_dir="$results_dir/fixtures"
mkdir -p "$fixtures_dir"
cmake -S "$source_dir" -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DIMGENGINE_BENCH=ON \
    -DIMGENGINE_LTO=OFF \
    -DIMGENGINE_ENABLE_DSL_CODEGEN=OFF
cmake --build "$build_dir" --target scheduler_c_baseline_bench imgengine --parallel
RUSTFLAGS="-L native=$build_dir" cargo build --release \
    --manifest-path "$source_dir/rust/imgengine-supervisor/Cargo.toml" \
    --bin scheduler_admission_bench

generate_fixtures() {
    convert -size 320x240 gradient:'#17324d-#e7b65a' "$fixtures_dir/small-jpeg.jpg"
    convert -size 320x240 gradient:'#17324d-#e7b65a' "$fixtures_dir/small-png.png"
    convert -size 1600x1200 plasma:fractal -colorspace sRGB -quality 88 "$fixtures_dir/representative-jpeg.jpg"
    convert -size 2048x1536 plasma:fractal -colorspace sRGB -quality 88 "$fixtures_dir/large-jpeg.jpg"
    convert -size 640x427 plasma:fractal -interlace Plane -quality 88 "$fixtures_dir/progressive-jpeg.jpg"
    printf '\377\330\377\340\000\020JFIF\000' > "$fixtures_dir/truncated-jpeg.jpg"
}
generate_fixtures

{
    echo "timestamp_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "git_revision=$git_revision"
    echo "git_worktree=$git_worktree"
    echo "container_os=$(grep PRETTY_NAME /etc/os-release | cut -d= -f2-)"
    echo "compiler=$(cc --version | head -n 1)"
    echo "cmake=$(cmake --version | head -n 1)"
    echo "rustc=$(rustc --version)"
    echo "cargo=$(cargo --version)"
    echo "kernel=$(uname -srmo)"
    echo "memory=$(grep MemTotal /proc/meminfo)"
    lscpu || true
    echo "iterations=$iterations"
    echo "warmup=$warmup"
    echo "concurrency=$concurrency"
    echo "queue_capacity=$queue_capacity"
    sha256sum "$fixtures_dir"/*
    for fixture in "$fixtures_dir"/*; do
        identify -format '%f %m %wx%h %b\n' "$fixture" 2>/dev/null ||
            printf '%s unreadable-intentionally\n' "$(basename "$fixture")"
    done
} > "$results_dir/environment.txt"

results="$results_dir/results.jsonl"
: > "$results"
c_bench="$build_dir/scheduler_c_baseline_bench"
rust_bench="$source_dir/rust/imgengine-supervisor/target/release/scheduler_admission_bench"

run_pair() {
    local fixture_name="$1"
    local fixture_path="$2"
    local mode="$3"
    local request_concurrency="$4"
    local request_queue_capacity="$5"
    LD_LIBRARY_PATH="$build_dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
        "$c_bench" --input "$fixture_path" --fixture "$fixture_name:$mode" \
        --iterations "$iterations" --warmup "$warmup" --concurrency "$request_concurrency" >> "$results"
    LD_LIBRARY_PATH="$build_dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
        RUSTFLAGS="-L native=$build_dir" \
        "$rust_bench" --input "$fixture_path" --fixture "$fixture_name:$mode" \
        --iterations "$iterations" --warmup "$warmup" --concurrency "$request_concurrency" \
        --queue-capacity "$request_queue_capacity" >> "$results"
}

for fixture in small-jpeg.jpg small-png.png representative-jpeg.jpg large-jpeg.jpg progressive-jpeg.jpg; do
    run_pair "$fixture" "$fixtures_dir/$fixture" sequential 1 "$queue_capacity"
done
run_pair cc0-camera-landscape.jpg "$camera_fixture" sequential 1 "$queue_capacity"
run_pair truncated-jpeg.jpg "$fixtures_dir/truncated-jpeg.jpg" sequential 1 "$queue_capacity"
for fixture in small-jpeg.jpg representative-jpeg.jpg large-jpeg.jpg; do
    run_pair "$fixture" "$fixtures_dir/$fixture" concurrent "$concurrency" "$queue_capacity"
done
run_pair cc0-camera-landscape.jpg "$camera_fixture" concurrent "$concurrency" "$queue_capacity"
saturation_concurrency=$((concurrency > 8 ? concurrency : 8))
run_pair small-jpeg.jpg "$fixtures_dir/small-jpeg.jpg" saturation "$saturation_concurrency" 1
if [[ -n "$real_image" ]]; then
    run_pair local-real-image "$real_image" sequential 1 "$queue_capacity"
fi

python3 - "$results" "$results_dir/summary.md" <<'PY'
import json
import sys
from pathlib import Path

records = [json.loads(line) for line in Path(sys.argv[1]).read_text().splitlines() if line]
lines = [
    "# Scheduler Characterization Results",
    "",
    "Results are host-specific evidence, not capacity or scale claims.",
    "",
    "| Workload | Path | P50 wall ms | P95 wall ms | P50 processing ms | Throughput ops/s | Success | Failed | Overload | Max queue | Peak RSS KiB |",
    "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|",
]
for record in records:
    def ms(value):
        return "n/a" if value is None else f"{value / 1_000_000:.3f}"
    lines.append(
        "| {fixture} | {path} | {p50} | {p95} | {processing} | {throughput:.2f} | {success} | {failed} | {overload} | {max_queue} | {rss} |".format(
            fixture=record["fixture"], path=record["path"],
            p50=ms(record["wall_latency_p50_ns"]), p95=ms(record["wall_latency_p95_ns"]),
            processing=ms(record["processing_latency_p50_ns"]),
            throughput=record["throughput_ops_per_sec"], success=record["successful_completions"],
            failed=record["failed_completions"], overload=record["queue_rejection_count"],
            max_queue=record["max_queue_depth"],
            rss=record["peak_rss_kib"],
        )
    )
Path(sys.argv[2]).write_text("\n".join(lines) + "\n")
PY

echo "Scheduler characterization evidence written to: $results_dir"
