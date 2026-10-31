#!/usr/bin/env bash
set -euo pipefail

base_url="${FRONTEND_URL:-http://127.0.0.1:3000}"
fixture="${1:-imgengine/photo.jpg}"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

for attempt in {1..45}; do
  if curl --fail --silent "$base_url/" >/dev/null; then
    break
  fi
  [[ "$attempt" -lt 45 ]] || { echo "Nuxt frontend did not become ready" >&2; exit 1; }
  sleep 1
done

headers="$work_dir/sheet.headers"
output="$work_dir/sheet.jpg"
status="$(curl --silent --show-error --output "$output" --dump-header "$headers" --write-out '%{http_code}' \
  -F "file=@$fixture;type=image/jpeg" \
  -F "sheet_width_mm=210" -F "sheet_height_mm=297" \
  -F "photo_width_mm=35" -F "photo_height_mm=45" -F "dpi=300" \
  -F "copies=8" -F "margin_mm=10" -F "gap_x_mm=2" -F "gap_y_mm=2" \
  -F "output_format=jpeg" \
  "$base_url/api/sheet-render")"

[[ "$status" == "200" ]] || {
  echo "Nuxt P12 proxy returned HTTP $status" >&2
  cat "$headers" >&2
  head -c 2000 "$output" >&2 || true
  exit 1
}

python3 - "$headers" "$output" <<'PY'
from pathlib import Path
import sys

headers_path, output_path = sys.argv[1:]
headers = {}
for line in Path(headers_path).read_text().splitlines():
    if ":" in line:
        key, value = line.split(":", 1)
        headers[key.strip().lower()] = value.strip()

data = Path(output_path).read_bytes()
assert headers.get("content-type") == "image/jpeg", headers
assert headers.get("x-request-id"), "Nuxt proxy dropped the Rust request ID"
assert headers.get("x-imgengine-canvas-width") == "2480", headers
assert headers.get("x-imgengine-canvas-height") == "3508", headers
assert headers.get("x-imgengine-expected-count") == "8", headers
assert headers.get("x-imgengine-planned-count") == "8", headers
assert headers.get("x-imgengine-executed-count") == "8", headers
assert data[:2] == b"\xff\xd8" and data[-2:] == b"\xff\xd9", "invalid JPEG framing"

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
        dimensions = (
            int.from_bytes(data[offset + 5:offset + 7], "big"),
            int.from_bytes(data[offset + 3:offset + 5], "big"),
        )
        break
    offset += length
assert dimensions == (2480, 3508), dimensions
PY

rejected_status="$(curl --silent --show-error --output "$work_dir/does-not-fit.json" --write-out '%{http_code}' \
  -F "file=@$fixture;type=image/jpeg" \
  -F "sheet_width_mm=210" -F "sheet_height_mm=297" \
  -F "photo_width_mm=35" -F "photo_height_mm=45" -F "dpi=300" \
  -F "copies=26" -F "margin_mm=10" -F "gap_x_mm=2" -F "gap_y_mm=2" \
  -F "output_format=jpeg" \
  "$base_url/api/sheet-render")"
[[ "$rejected_status" == "422" ]] || { echo "Expected Nuxt P12 capacity rejection (422), got $rejected_status" >&2; cat "$work_dir/does-not-fit.json" >&2; exit 1; }
python3 - "$work_dir/does-not-fit.json" <<'PY'
import json, sys
problem = json.load(open(sys.argv[1], encoding="utf-8"))
assert problem.get("status") == 422, problem
assert problem.get("code") == "does_not_fit", problem
assert problem.get("request_id"), problem
PY

echo "Nuxt proxy smoke passed: P12 JPEG success, server accounting, and exact-copy rejection"
