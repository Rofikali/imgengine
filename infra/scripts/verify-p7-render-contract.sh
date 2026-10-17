#!/usr/bin/env bash
set -euo pipefail

fixture=/workspace/imgengine/imgengine/tests/fixtures/cc0_camera_landscape.jpg
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
cp "$fixture" "$tmpdir/input.jpg"
# Keep the PNG fixture below the documented provisional 1 MiB HTTP cap. The
# full source JPEG remains the real JPEG path; a separate case exercises 413.
convert "$tmpdir/input.jpg" -resize 128x128 "$tmpdir/input.png"

render() {
  curl --silent --show-error --output "$2" --write-out '%{http_code}' \
    -F "file=@$1;type=$3" http://localhost:3000/api/render
}

[[ $(render "$tmpdir/input.jpg" "$tmpdir/output-from-jpeg.jpg" image/jpeg) == 200 ]]
[[ $(render "$tmpdir/input.png" "$tmpdir/output-from-png.jpg" image/png) == 200 ]]
[[ $(identify -format '%m' "$tmpdir/output-from-jpeg.jpg") == JPEG ]]
[[ $(identify -format '%m' "$tmpdir/output-from-png.jpg") == JPEG ]]

[[ $(curl --silent --output /dev/null --write-out '%{http_code}' -X POST http://localhost:3000/api/render) == 400 ]]
printf 'not an image' > "$tmpdir/unsupported.txt"
[[ $(render "$tmpdir/unsupported.txt" /dev/null text/plain) == 415 ]]
printf 'not a JPEG' > "$tmpdir/malformed.jpg"
[[ $(render "$tmpdir/malformed.jpg" /dev/null image/jpeg) == 422 ]]
head -c 1100000 /dev/zero > "$tmpdir/oversized.jpg"
[[ $(render "$tmpdir/oversized.jpg" /dev/null image/jpeg) == 413 ]]
[[ $(curl --silent --output /dev/null --write-out '%{http_code}' \
  -F "file=@$tmpdir/input.jpg;type=image/jpeg" http://rust-api:8000/api/v1/render) == 401 ]]

echo 'P7 Nuxt-to-Rust render contract passed'
