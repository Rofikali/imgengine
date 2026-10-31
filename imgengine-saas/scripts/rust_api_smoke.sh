#!/usr/bin/env bash
set -euo pipefail

api_url="${API_URL:-http://127.0.0.1:8001}"
api_key="${API_KEY:?API_KEY must be set}"
fixture="${1:-imgengine/photo.jpg}"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

for attempt in {1..45}; do
  if curl --fail --silent "$api_url/readyz" >/dev/null; then
    break
  fi
  [[ "$attempt" -lt 45 ]] || { echo "Rust API did not become ready" >&2; exit 1; }
  sleep 1
done

check_jpeg() {
  local name="$1"
  local expected_width="$2"
  local expected_height="$3"
  local expected_count="$4"
  local headers="$work_dir/$name.headers"
  local output="$work_dir/$name.jpg"

  python3 - "$headers" "$output" "$expected_width" "$expected_height" "$expected_count" <<'PY'
from pathlib import Path
import sys

headers_path, output_path, expected_width, expected_height, count = sys.argv[1:]
headers = {}
for line in Path(headers_path).read_text().splitlines():
    if ":" in line:
        key, value = line.split(":", 1)
        headers[key.strip().lower()] = value.strip()

data = Path(output_path).read_bytes()
assert data[:2] == b"\xff\xd8" and data[-2:] == b"\xff\xd9", "invalid JPEG framing"
assert headers.get("content-type") == "image/jpeg", headers
assert headers.get("x-request-id"), "missing request ID"
if expected_width != "0":
    assert headers.get("x-imgengine-canvas-width") == expected_width, headers
    assert headers.get("x-imgengine-canvas-height") == expected_height, headers
    assert headers.get("x-imgengine-expected-count") == count, headers
    assert headers.get("x-imgengine-planned-count") == count, headers
    assert headers.get("x-imgengine-executed-count") == count, headers

# Parse JPEG SOF markers without requiring Pillow or ImageMagick on the host.
offset = 2
dimensions = None
while offset < len(data):
    if data[offset] != 0xFF:
        offset += 1
        continue
    while offset < len(data) and data[offset] == 0xFF:
        offset += 1
    if offset >= len(data):
        break
    marker = data[offset]
    offset += 1
    if marker in (0xD8, 0xD9, 0x01) or 0xD0 <= marker <= 0xD7:
        continue
    if offset + 2 > len(data):
        break
    length = int.from_bytes(data[offset:offset + 2], "big")
    if length < 2 or offset + length > len(data):
        break
    if marker in (0xC0, 0xC1, 0xC2, 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF):
        actual_height = int.from_bytes(data[offset + 3:offset + 5], "big")
        actual_width = int.from_bytes(data[offset + 5:offset + 7], "big")
        dimensions = (actual_width, actual_height)
        break
    offset += length
assert dimensions == (int(expected_width), int(expected_height)) if expected_width != "0" else dimensions is not None, dimensions
PY
}

curl --fail --silent --show-error -D "$work_dir/render.headers" \
  -H "X-API-Key: $api_key" \
  -F "file=@$fixture;type=image/jpeg" \
  "$api_url/api/v1/render" -o "$work_dir/render.jpg"
check_jpeg render 0 0 0

curl --fail --silent --show-error -D "$work_dir/layout.headers" \
  -H "X-API-Key: $api_key" \
  -F "file=@$fixture;type=image/jpeg" \
  -F "width=12" -F "height=8" -F "unit=px" -F "copies=2" \
  -F "columns=2" -F "gap=2" -F "margin=1" -F "output_format=jpeg" \
  "$api_url/api/v1/layout-render" -o "$work_dir/layout.jpg"
check_jpeg layout 28 10 2

curl --fail --silent --show-error -D "$work_dir/sheet.headers" \
  -H "X-API-Key: $api_key" \
  -F "file=@$fixture;type=image/jpeg" \
  -F "sheet_width_mm=210" -F "sheet_height_mm=297" \
  -F "photo_width_mm=35" -F "photo_height_mm=45" -F "dpi=300" \
  -F "copies=8" -F "margin_mm=10" -F "gap_x_mm=2" -F "gap_y_mm=2" \
  -F "output_format=jpeg" \
  "$api_url/api/v2/sheet-render" -o "$work_dir/sheet.jpg"
check_jpeg sheet 2480 3508 8

rejected_status="$(curl --silent --show-error --output "$work_dir/does-not-fit.json" --write-out '%{http_code}' \
  -H "X-API-Key: $api_key" \
  -F "file=@$fixture;type=image/jpeg" \
  -F "sheet_width_mm=210" -F "sheet_height_mm=297" \
  -F "photo_width_mm=35" -F "photo_height_mm=45" -F "dpi=300" \
  -F "copies=26" -F "margin_mm=10" -F "gap_x_mm=2" -F "gap_y_mm=2" \
  -F "output_format=jpeg" \
  "$api_url/api/v2/sheet-render")"
[[ "$rejected_status" == "422" ]] || { echo "Expected P12 capacity rejection (422), got $rejected_status" >&2; cat "$work_dir/does-not-fit.json" >&2; exit 1; }
python3 - "$work_dir/does-not-fit.json" <<'PY'
import json, sys
problem = json.load(open(sys.argv[1], encoding="utf-8"))
assert problem.get("status") == 422, problem
assert problem.get("code") == "does_not_fit", problem
assert problem.get("request_id"), problem
PY

echo "Rust API smoke passed: P5 render, P8 FIT layout, P12 A4 sheet and exact-copy rejection"
