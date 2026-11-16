#!/usr/bin/env bash
set -euo pipefail
# P8-SIM-02 P12/P13 HTTP smoke inside the isolated workload container.
# Deep pixel-parity/orientation oracle remains in docs/P13_M1_3_CROP_PARITY.md.
if [[ "$#" -ne 2 || -z "$1" || -z "$2" ]]; then
    echo 'usage: run-p8-vps-sheet-crop-workload.sh REPORT JPEG_FIXTURE' >&2
    exit 2
fi
report=$1
fixture=$2
: "$P8_SHEET_URL"
workdir=$(mktemp -d)
trap 'rm -rf "$workdir"' EXIT
# ImageMagick resize normally preserves source aspect; force square geometry.
convert "$fixture" -resize '128x128!' "$workdir/square.png"
success=0
rejected=0

request() {
    local label=$1 expected=$2
    shift 2
    local status dims count dpi
    dpi=100
    if [[ "$label" == p12_oversized_dpi ]]; then dpi=1201; fi
    status=$(curl --silent --show-error --output "$workdir/$label.body" \
        --dump-header "$workdir/$label.headers" --write-out '%{http_code}' \
        -F "file=@$workdir/square.png;type=image/png" \
        -F sheet_width_mm=25.4 -F sheet_height_mm=25.4 \
        -F photo_width_mm=25.4 -F photo_height_mm=25.4 \
        -F "dpi=$dpi" -F copies=1 -F margin_mm=0 \
        -F gap_x_mm=0 -F gap_y_mm=0 -F output_format=jpeg \
        "$@" "$P8_SHEET_URL")
    if [[ "$status" != "$expected" ]]; then
        echo "$label: HTTP $status expected $expected" >&2
        return 1
    fi
    if [[ "$expected" == 200 ]]; then
        dims=$(identify -format '%m %wx%h' "$workdir/$label.body")
        if [[ "$dims" != 'JPEG 100x100' ]]; then
            echo "$label: JPEG dimensions wrong: $dims" >&2; return 1
        fi
        count=$(awk -F ': *' 'tolower($1)=="x-imgengine-executed-count" {gsub("\r","",$2); print $2}' "$workdir/$label.headers" | tail -n 1)
        if [[ "$count" != 1 ]]; then
            echo "$label: missing expected executed-count header" >&2; return 1
        fi
        ((success+=1))
    else
        if ! grep -qi '^content-type: application/problem+json' "$workdir/$label.headers"; then
            echo "$label: expected structured problem JSON" >&2; return 1
        fi
        ((rejected+=1))
    fi
}
# P12 legacy behavior: crop fields completely omitted.
request p12_no_crop 200
# P13 crop: top-right quarter of the stored 128x128 raster, 64x64 source pixels.
request p13_crop 200 -F crop_x=500000 -F crop_y=0 \
    -F crop_width=500000 -F crop_height=500000
if cmp -s "$workdir/p12_no_crop.body" "$workdir/p13_crop.body"; then
    echo 'P13 selected-crop JPEG unexpectedly identical to centered P12 output' >&2
    exit 1
fi
request p13_partial 400 -F crop_x=0
request p13_duplicate 400 -F crop_x=0 -F crop_x=0 -F crop_y=0 \
    -F crop_width=1000000 -F crop_height=1000000
request p13_negative 400 -F crop_x=-1 -F crop_y=0 \
    -F crop_width=1000000 -F crop_height=1000000
request p13_exponent 400 -F crop_x=0 -F crop_y=0 \
    -F crop_width=1e6 -F crop_height=1000000
request p13_out_of_range 400 -F crop_x=500000 -F crop_y=0 \
    -F crop_width=500001 -F crop_height=1000000
# Excessive physical dimensions rejected at the Rust resource boundary.
request p12_oversized_dpi 413

printf '{"schema":"imgengine.p8.sheet-crop.v1","successful_cases":%d,"rejected_cases":%d,"coverage":"P12 omission; P13 selected crop; partial/duplicate/negative/exponent/out-of-range; DPI resource bound; JPEG geometry; executed count; problem media type"}\n' \
    "$success" "$rejected" >"$report"
