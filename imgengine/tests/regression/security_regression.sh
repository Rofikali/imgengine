#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <build_dir> [tmp_dir]" >&2
    exit 2
fi

build_dir="$1"
tmp_dir="${2:-${build_dir}/security_regression_tmp}"
cli="${build_dir}/imgengine_cli"

command -v python3 >/dev/null || { echo "python3 is required" >&2; exit 3; }
[[ -x "$cli" ]] || { echo "Missing CLI: $cli" >&2; exit 4; }
mkdir -p "$tmp_dir"

python3 - "$tmp_dir" <<'PY'
import struct
import sys
import zlib
from pathlib import Path

directory = Path(sys.argv[1])
(directory / "random.bin").write_bytes(b"not-an-image\x00\xff\x01")
(directory / "truncated.jpg").write_bytes(b"\xff\xd8\xff\xe0\x00\x10JFIF\x00")

ihdr = struct.pack(">IIBBBBB", 20000, 1, 8, 2, 0, 0, 0)
chunk = b"IHDR" + ihdr
png = b"\x89PNG\r\n\x1a\n" + struct.pack(">I", len(ihdr)) + chunk
png += struct.pack(">I", zlib.crc32(chunk) & 0xFFFFFFFF)
png += struct.pack(">I", 0) + b"IEND" + struct.pack(">I", zlib.crc32(b"IEND") & 0xFFFFFFFF)
(directory / "oversized.png").write_bytes(png)
PY

assert_rejected() {
    local fixture="$1"
    local expected_error="$2"
    local output="$tmp_dir/$(basename "$fixture").out.jpg"
    local log="$tmp_dir/$(basename "$fixture").log"

    rm -f "$output"
    if "$cli" --quiet --input "$fixture" --output "$output" >"$log" 2>&1; then
        echo "Expected rejection for $fixture" >&2
        exit 5
    fi
    grep -Eq "job failed: (${expected_error}) \\([0-9]+\\)" "$log" || {
        echo "Expected ${expected_error} for $fixture" >&2
        cat "$log" >&2
        exit 6
    }
    [[ ! -s "$output" ]] || {
        echo "Rejected input produced an output artifact: $output" >&2
        exit 7
    }
}

assert_rejected "$tmp_dir/random.bin" IMG_ERR_FORMAT
assert_rejected "$tmp_dir/truncated.jpg" 'IMG_ERR_FORMAT|IMG_ERR_SECURITY'
assert_rejected "$tmp_dir/oversized.png" IMG_ERR_SECURITY

echo "OK: hostile input regressions passed"
