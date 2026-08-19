#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <optimized_cli> <portable_cli> [tmp_dir]" >&2
    exit 2
fi

optimized_cli="$1"
portable_cli="$2"
tmp_dir="${3:-$(dirname "$optimized_cli")/scalar_equivalence_tmp}"
raw_fixture="${tmp_dir}/odd-gradient.rgb"

command -v compare >/dev/null || { echo "ImageMagick compare is required" >&2; exit 3; }
command -v python3 >/dev/null || { echo "python3 is required" >&2; exit 3; }
[[ -x "$optimized_cli" && -x "$portable_cli" ]] || { echo "Both CLI binaries are required" >&2; exit 4; }
mkdir -p "$tmp_dir"

python3 - "$raw_fixture" <<'PY'
import sys

with open(sys.argv[1], "wb") as fixture:
    for y in range(53):
        for x in range(97):
            fixture.write(bytes(((x * 17 + y * 3) % 256, (x * 5 + y * 19) % 256, (x * 11 + y * 7) % 256)))
PY

common_args=(--quiet --input "$raw_fixture" --input-format raw-rgb24 --input-width 97 --input-height 53
             --cols 2 --rows 2 --width 3 --height 3 --dpi 30 --padding 12 --gap 7
             --border 2 --bleed 1 --crop-mark 4 --crop-thickness 1 --crop-offset 2 --mode fill)
"$optimized_cli" "${common_args[@]}" --output "$tmp_dir/optimized.jpg"
"$portable_cli" "${common_args[@]}" --output "$tmp_dir/portable.jpg"

different_pixels="$(compare -metric AE "$tmp_dir/optimized.jpg" "$tmp_dir/portable.jpg" null: 2>&1 || true)"
awk -v changed="$different_pixels" 'BEGIN { exit !(changed + 0 == 0) }' || {
    echo "Optimized and portable outputs differ: ${different_pixels} pixels" >&2
    exit 5
}

echo "OK: optimized and portable scalar outputs are pixel-equivalent"
