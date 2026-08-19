#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <build_dir> [tmp_dir]" >&2
    exit 2
fi

build_dir="$1"
tmp_dir="${2:-${build_dir}/geometry_regression_tmp}"
cli="${build_dir}/imgengine_cli"
raw_fixture="${tmp_dir}/wide-gradient.rgb"

command -v compare >/dev/null || { echo "ImageMagick compare is required" >&2; exit 3; }
command -v identify >/dev/null || { echo "ImageMagick identify is required" >&2; exit 3; }
command -v python3 >/dev/null || { echo "python3 is required" >&2; exit 3; }
[[ -x "$cli" ]] || { echo "Missing CLI: $cli" >&2; exit 4; }
mkdir -p "$tmp_dir"

python3 - "$raw_fixture" <<'PY'
import sys

path = sys.argv[1]
width, height = 96, 48
with open(path, "wb") as fixture:
    for _ in range(height):
        for x in range(width):
            fixture.write(bytes((255 - (x * 255 // (width - 1)), 48, x * 255 // (width - 1))))
PY

run_layout() {
    local output="$1"
    shift
    "$cli" --quiet --input "$raw_fixture" --input-format raw-rgb24 \
        --input-width 96 --input-height 48 --cols 1 --rows 1 \
        --width 3 --height 3 --dpi 30 --padding 20 "$@" --output "$output"
    [[ -s "$output" ]] || { echo "Expected output: $output" >&2; exit 5; }
    [[ "$(identify -format '%wx%h' "$output")" == "248x351" ]] || {
        echo "Unexpected A4 output dimensions for $output" >&2
        exit 6
    }
}

assert_different() {
    local left="$1"
    local right="$2"
    local changed
    changed="$(compare -metric AE "$left" "$right" null: 2>&1 || true)"
    awk -v changed="$changed" 'BEGIN { exit !(changed + 0 > 0) }' || {
        echo "Expected raster difference between $left and $right" >&2
        exit 7
    }
}

run_layout "$tmp_dir/fill.jpg" --mode fill --border 0 --bleed 0 --crop-mark 0
run_layout "$tmp_dir/fit.jpg" --mode fit --border 0 --bleed 0 --crop-mark 0
run_layout "$tmp_dir/border.jpg" --mode fill --border 3 --bleed 0 --crop-mark 0
run_layout "$tmp_dir/bleed.jpg" --mode fill --border 0 --bleed 3 --crop-mark 0
run_layout "$tmp_dir/crop.jpg" --mode fill --border 0 --bleed 0 --crop-mark 10 --crop-thickness 2 --crop-offset 4

assert_different "$tmp_dir/fill.jpg" "$tmp_dir/fit.jpg"
assert_different "$tmp_dir/fill.jpg" "$tmp_dir/border.jpg"
assert_different "$tmp_dir/fill.jpg" "$tmp_dir/bleed.jpg"
assert_different "$tmp_dir/fill.jpg" "$tmp_dir/crop.jpg"

echo "OK: layout geometry regressions passed"
