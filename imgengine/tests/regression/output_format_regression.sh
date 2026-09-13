#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <build_dir> [tmp_dir]" >&2
    exit 2
fi

build_dir="$1"
tmp_dir="${2:-${build_dir}/output_format_regression_tmp}"
cli="${build_dir}/imgengine_cli"

command -v python3 >/dev/null || { echo "python3 is required" >&2; exit 3; }
[[ -x "$cli" ]] || { echo "Missing CLI: $cli" >&2; exit 4; }

rm -rf "$tmp_dir"
mkdir -p "$tmp_dir"

python3 - "$tmp_dir/input.png" <<'PY'
import struct
import sys
import zlib

path = sys.argv[1]

def chunk(kind, data):
    return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff)

png = b"\x89PNG\r\n\x1a\n"
png += chunk(b"IHDR", struct.pack(">IIBBBBB", 1, 1, 8, 2, 0, 0, 0))
png += chunk(b"IDAT", zlib.compress(b"\x00\x40\x80\xc0"))
png += chunk(b"IEND", b"")
open(path, "wb").write(png)
PY

input="$tmp_dir/input.png"
jpeg_output="$tmp_dir/layout.jpeg"
pdf_output="$tmp_dir/layout.pdf"
png_output="$tmp_dir/layout.png"

"$cli" --quiet --input "$input" --output "$jpeg_output" --cols 3 --rows 2
[[ "$(head -c 2 "$jpeg_output")" == $'\xff\xd8' ]] || {
    echo "Expected JPEG signature for .jpeg output" >&2
    exit 5
}

"$cli" --quiet --input "$input" --output "$pdf_output" --cols 3 --rows 2
[[ "$(head -c 5 "$pdf_output")" == "%PDF-" ]] || {
    echo "Expected PDF signature for .pdf output" >&2
    exit 6
}

if "$cli" --quiet --input "$input" --output "$png_output" >"$tmp_dir/png.log" 2>&1; then
    echo "Expected .png output rejection" >&2
    exit 7
fi
grep -Fq "unsupported output extension; use .jpg, .jpeg, or .pdf" "$tmp_dir/png.log" || {
    cat "$tmp_dir/png.log" >&2
    exit 8
}
[[ ! -e "$png_output" ]] || {
    echo "Unsupported .png output created an artifact" >&2
    exit 9
}

echo "OK: output format contract passed"
