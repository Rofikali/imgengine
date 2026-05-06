#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <build_dir> [tmp_dir]" >&2
  exit 2
fi

BUILD_DIR="$1"
TMPDIR="${2:-${BUILD_DIR}/regression_tmp}"
mkdir -p "$TMPDIR"

echo "Downloading sample image..."
curl -sSL -o "$TMPDIR/sample.jpg" https://upload.wikimedia.org/wikipedia/commons/3/3f/Fronalpstock_big.jpg

if command -v convert >/dev/null 2>&1; then
  echo "Creating progressive JPEG with ImageMagick..."
  convert "$TMPDIR/sample.jpg" -interlace JPEG "$TMPDIR/prog.jpg"
elif command -v jpegtran >/dev/null 2>&1; then
  echo "Creating progressive JPEG with jpegtran..."
  jpegtran -progressive -outfile "$TMPDIR/prog.jpg" "$TMPDIR/sample.jpg"
else
  echo "Error: need ImageMagick 'convert' or 'jpegtran' to make progressive JPEG" >&2
  exit 3
fi

CLI="$BUILD_DIR/imgengine_cli"
if [ ! -x "$CLI" ]; then
  CLI="$BUILD_DIR/imgengine_cli"
fi

OUT="$TMPDIR/out.jpg"

echo "Running imgengine_cli on progressive JPEG..."
"$CLI" --input "$TMPDIR/prog.jpg" --output "$OUT" --cols 6 --rows 3 --gap 20 --padding 20

if [ ! -f "$OUT" ]; then
  echo "Error: expected output file not produced" >&2
  exit 4
fi

echo "OK: progressive regression passed, output=$OUT"
exit 0
