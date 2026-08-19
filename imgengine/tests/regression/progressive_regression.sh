#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -lt 1 ]; then
  echo "Usage: $0 <build_dir> [tmp_dir]" >&2
  exit 2
fi

BUILD_DIR="$1"
TMPDIR="${2:-${BUILD_DIR}/regression_tmp}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_ROOT="${IMGENGINE_SOURCE_DIR:-${SCRIPT_DIR}/../..}"
SOURCE_IMAGE="${SOURCE_ROOT}/photo.jpg"
mkdir -p "$TMPDIR"

[ -f "$SOURCE_IMAGE" ] || { echo "Missing regression fixture: $SOURCE_IMAGE" >&2; exit 3; }

if command -v convert >/dev/null 2>&1; then
  echo "Creating progressive and CMYK JPEG fixtures with ImageMagick..."
  convert "$SOURCE_IMAGE" -strip -interlace JPEG "$TMPDIR/progressive.jpg"
  convert "$SOURCE_IMAGE" -strip -colorspace CMYK -interlace JPEG "$TMPDIR/cmyk.jpg"
else
  echo "Error: need ImageMagick 'convert' to create JPEG regression fixtures" >&2
  exit 4
fi

CLI="$BUILD_DIR/imgengine_cli"
if [ ! -x "$CLI" ]; then
  CLI="$BUILD_DIR/imgengine_cli"
fi

for fixture in progressive cmyk; do
  output="$TMPDIR/${fixture}-out.jpg"
  echo "Running imgengine_cli on ${fixture} JPEG..."
  "$CLI" --input "$TMPDIR/${fixture}.jpg" --output "$output" --cols 6 --rows 3 --gap 20 --padding 20
  [ -s "$output" ] || { echo "Error: expected output for ${fixture} fixture" >&2; exit 5; }
done

echo "OK: progressive and CMYK JPEG regressions passed"
exit 0
