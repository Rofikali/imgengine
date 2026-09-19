#!/usr/bin/env bash
set -euo pipefail

build_dir="$(cd "$1" && pwd)"
source_dir="$(cd "$2" && pwd)"
mode="${3:-all}"
fixture_dir="$build_dir/real-image-fixtures"
report="$build_dir/real-image-verification.md"
consumer="$build_dir/test_abi_real_image_consumer"
repetitions="${IMGENGINE_REAL_IMAGE_REPETITIONS:-10}"

rm -rf "$fixture_dir"
mkdir -p "$fixture_dir/c" "$fixture_dir/rust"

magick_cmd=(convert)
if command -v magick >/dev/null 2>&1; then
    magick_cmd=(magick)
fi

"${magick_cmd[@]}" -size 320x240 gradient: -colorspace sRGB -strip "$fixture_dir/small-rgb.jpg"
"${magick_cmd[@]}" -size 2048x1536 plasma:fractal -colorspace sRGB -quality 90 "$fixture_dir/large-rgb.jpg"
"${magick_cmd[@]}" -size 333x127 gradient: -alpha set -channel A -evaluate set 70% +channel "$fixture_dir/alpha.png"
"${magick_cmd[@]}" -size 513x257 gradient: -colorspace Gray "$fixture_dir/grayscale.jpg"
"${magick_cmd[@]}" -size 640x427 plasma:fractal -interlace Plane "$fixture_dir/progressive.jpg"
"${magick_cmd[@]}" -size 640x427 gradient: -colorspace CMYK "$fixture_dir/cmyk.jpg"
"${magick_cmd[@]}" -size 17x701 gradient: -colorspace sRGB "$fixture_dir/tall.png"
"${magick_cmd[@]}" -size 16385x1 xc:red -quality 90 "$fixture_dir/oversized-dimension.jpg"
head -c 20 "$fixture_dir/small-rgb.jpg" > "$fixture_dir/truncated.jpg"
head -c 20 "$fixture_dir/alpha.png" > "$fixture_dir/truncated.png"
: > "$fixture_dir/empty.bin"
printf 'not an image\n' > "$fixture_dir/unsupported.bin"

cat > "$report" <<EOF
# Real-Image Verification

- Generated on: $(date -u +%Y-%m-%dT%H:%M:%SZ)
- Consumer interface: sealed public libimgengine ABI v1 only
- Fixture source: deterministic ImageMagick generation in Ubuntu 24.04; optional local images are not retained or staged
- Repetitions per successful input: ${repetitions}

| Fixture | Input metadata | C ABI | Rust FFI | C output metadata | Rust output metadata |
|---|---|---|---|---|---|
EOF

run_valid() {
    local fixture="$1"
    local input="${2:-$fixture_dir/$fixture}"
    local c_output="$fixture_dir/c/$fixture.jpg"
    local rust_output="$fixture_dir/rust/$fixture.jpg"
    local input_info c_info rust_info c_result rust_result
    input_info=$(identify -format '%m %wx%h %b' "$input")
    c_result=$("$consumer" "$input" "$c_output" 0 "$repetitions")
    c_info=$(identify -format '%m %wx%h %b' "$c_output")
    rust_result="not-run"
    rust_info="not-run"
    if [[ "$mode" == "all" ]]; then
        rust_result=$(LD_LIBRARY_PATH="$build_dir${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
            RUSTFLAGS="-L native=$build_dir" \
            cargo run --quiet --manifest-path "$source_dir/rust/imgengine-ffi-smoke/Cargo.toml" -- \
            "$input" "$rust_output" "$repetitions")
        rust_info=$(identify -format '%m %wx%h %b' "$rust_output")
    fi
    printf '| `%s` | %s | `%s` | `%s` | %s | %s |\n' "$fixture" "$input_info" "$c_result" \
        "$rust_result" "$c_info" "$rust_info" >> "$report"
}

run_invalid() {
    local fixture="$1"
    local expected="$2"
    local result
    result=$("$consumer" "$fixture_dir/$fixture" "$fixture_dir/c/$fixture.out" "$expected" 1)
    printf '| `%s` | invalid input | `%s` | n/a | n/a | n/a |\n' "$fixture" "$result" >> "$report"
}

for fixture in small-rgb.jpg large-rgb.jpg alpha.png grayscale.jpg progressive.jpg cmyk.jpg tall.png; do
    run_valid "$fixture"
done
for local_fixture in photo.jpg photo.png; do
    if [[ -f "$source_dir/$local_fixture" ]]; then
        run_valid "local-$local_fixture" "$source_dir/$local_fixture"
    fi
done
run_invalid truncated.jpg 3
run_invalid truncated.png 2
run_invalid empty.bin 1
run_invalid unsupported.bin 2
run_invalid oversized-dimension.jpg 3

cat "$report"
