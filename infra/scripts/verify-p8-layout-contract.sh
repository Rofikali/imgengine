#!/usr/bin/env bash
set -euo pipefail

# Exercise the browser-facing Nuxt proxy, Rust HTTP adapter, lifecycle,
# bounded admission, additive native ABI, retained C scheduler, and JPEG output
# with project-owned fixtures. This is a Docker development/integration gate;
# the Ubuntu native verifier remains the authoritative release gate.
fixture=/workspace/imgengine/imgengine/tests/fixtures/cc0_camera_landscape.jpg
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT
cp "$fixture" "$tmpdir/input.jpg"
convert "$tmpdir/input.jpg" -resize 128x128 "$tmpdir/input.png"

layout() {
  curl --silent --show-error --output "$2" --write-out '%{http_code}' \
    -F "file=@$1;type=$3" \
    -F width=12 -F height=8 -F unit=px -F copies=2 -F columns=2 \
    -F gap=2 -F margin=1 -F output_format=jpeg \
    http://localhost:3000/api/layout-render
}

[[ $(layout "$tmpdir/input.jpg" "$tmpdir/from-jpeg.jpg" image/jpeg) == 200 ]]
[[ $(layout "$tmpdir/input.png" "$tmpdir/from-png.jpg" image/png) == 200 ]]
[[ $(identify -format '%m %w %h' "$tmpdir/from-jpeg.jpg") == 'JPEG 28 10' ]]
[[ $(identify -format '%m %w %h' "$tmpdir/from-png.jpg") == 'JPEG 28 10' ]]

[[ $(curl --silent --output /dev/null --write-out '%{http_code}' \
  -F "file=@$tmpdir/input.jpg;type=image/jpeg" -F width=12 -F height=8 \
  -F unit=px -F copies=1 -F gap=0 -F margin=0 -F output_format=png \
  http://localhost:3000/api/layout-render) == 400 ]]
[[ $(curl --silent --output /dev/null --write-out '%{http_code}' \
  -F "file=@$tmpdir/input.jpg;type=image/jpeg" -F width=99999 -F height=8 \
  -F unit=px -F copies=1 -F gap=0 -F margin=0 -F output_format=jpeg \
  http://localhost:3000/api/layout-render) == 413 ]]

echo 'P8 Nuxt-to-Rust FIT-only layout contract passed'
