#!/usr/bin/env bash
set -euo pipefail

# Runs inside the frontend service of the constrained Compose profile. Fixtures
# are project-owned; all generated images and response bodies live in /tmp.
results_file=${1:?usage: run-p8-vps-workload.sh RESULTS_FILE}
layout_url=${P8_LAYOUT_URL:?P8_LAYOUT_URL must name the same-origin proxy endpoint}
fixture=/opt/imgengine-fixtures/cc0_camera_landscape.jpg
workdir=$(mktemp -d)
trap 'rm -rf "$workdir"' EXIT
workload_started_ns=$(date +%s%N)

cp "$fixture" "$workdir/input.jpg"
convert "$workdir/input.jpg" -resize 128x128 "$workdir/input.png"
printf 'not an image\n' > "$workdir/malformed.bin"

declare -a labels statuses times

request() {
  local label=$1 input=$2 mime=$3
  shift 3
  local body="$workdir/$label.body" headers="$workdir/$label.headers" timing
  timing=$(curl --silent --show-error --output "$body" --dump-header "$headers" \
    --write-out '%{http_code} %{time_total}' \
    -F "file=@$input;type=$mime" "$@" "$layout_url")
  local status=${timing%% *} elapsed=${timing#* }
  labels+=("$label")
  statuses+=("$status")
  times+=("$elapsed")
  if [[ "$status" == 200 ]]; then
    identify -format '%m %w %h' "$body" >/dev/null
  fi
}

layout_fields=(
  -F width=12 -F height=8 -F unit=px -F copies=2 -F columns=2
  -F gap=2 -F margin=1 -F output_format=jpeg
)
request valid_jpeg "$workdir/input.jpg" image/jpeg "${layout_fields[@]}"
request valid_png "$workdir/input.png" image/png "${layout_fields[@]}"
request copies_gap_margin "$workdir/input.jpg" image/jpeg \
  -F width=20 -F height=15 -F unit=px -F copies=6 -F columns=3 \
  -F gap=3 -F margin=2 -F output_format=jpeg
request invalid_output "$workdir/input.jpg" image/jpeg \
  -F width=12 -F height=8 -F unit=px -F copies=1 -F gap=0 -F margin=0 -F output_format=png
# Deliberately label malformed bytes as JPEG so this reaches decoder validation
# rather than the earlier declared-media policy and demonstrates the 422 path.
request malformed "$workdir/malformed.bin" image/jpeg "${layout_fields[@]}"
request excessive_plan "$workdir/input.jpg" image/jpeg \
  -F width=4096 -F height=4096 -F unit=px -F copies=1 -F gap=0 -F margin=0 -F output_format=jpeg
request missing_required "$workdir/input.jpg" image/jpeg \
  -F height=8 -F unit=px -F copies=1 -F gap=0 -F margin=0 -F output_format=jpeg

expect_status() {
  local label=$1 expected=$2 index
  for index in "${!labels[@]}"; do
    if [[ "${labels[$index]}" == "$label" ]]; then
      [[ "${statuses[$index]}" == "$expected" ]]
      return
    fi
  done
  echo "missing workload result for $label" >&2
  return 1
}

expect_status valid_jpeg 200
expect_status valid_png 200
expect_status copies_gap_margin 200
expect_status invalid_output 400
expect_status malformed 422
expect_status excessive_plan 413
expect_status missing_required 400

for index in $(seq 1 10); do
  request "warm_${index}" "$workdir/input.jpg" image/jpeg "${layout_fields[@]}"
done

# A bounded concurrent burst characterizes the existing one-slot admission
# contract. It deliberately permits either completion or documented 429; it
# does not assume a timing-sensitive ordering.
for index in 1 2 3 4; do
  (
    curl --silent --show-error --output "$workdir/burst_${index}.body" \
      --write-out '%{http_code} %{time_total}' \
      -F "file=@$workdir/input.jpg;type=image/jpeg" "${layout_fields[@]}" \
      "$layout_url" > "$workdir/burst_${index}.result"
  ) &
done
wait
for index in 1 2 3 4; do
  value=$(<"$workdir/burst_${index}.result")
  labels+=("burst_${index}")
  statuses+=("${value%% *}")
  times+=("${value#* }")
done

success=0 rejected=0 failed=0
for status in "${statuses[@]}"; do
  case "$status" in
    200) ((success+=1)) ;;
    400|413|415|422|429|503|504) ((rejected+=1)) ;;
    *) ((failed+=1)) ;;
  esac
done

workload_finished_ns=$(date +%s%N)
workload_duration_seconds=$(awk -v start="$workload_started_ns" -v end="$workload_finished_ns" \
  'BEGIN { printf "%.6f", (end - start) / 1000000000 }')
throughput_requests_per_second=$(awk -v count="${#labels[@]}" -v duration="$workload_duration_seconds" \
  'BEGIN { if (duration > 0) printf "%.6f", count / duration; else print "0.000000" }')

percentile() {
  local fraction=$1 count=${#times[@]}
  printf '%s\n' "${times[@]}" | sort -n | awk -v n="$count" -v f="$fraction" \
    'BEGIN { target = int((n - 1) * f + 1.5) } NR == target { print; exit }'
}

{
  printf '{\n'
  printf '  "request_count": %d,\n' "${#labels[@]}"
  printf '  "success_count": %d,\n' "$success"
  printf '  "rejected_count": %d,\n' "$rejected"
  printf '  "failed_count": %d,\n' "$failed"
  printf '  "workload_duration_seconds": %s,\n' "$workload_duration_seconds"
  printf '  "throughput_requests_per_second": %s,\n' "$throughput_requests_per_second"
  printf '  "latency_seconds": {"p50": %s, "p95": %s, "p99": %s},\n' \
    "$(percentile 0.50)" "$(percentile 0.95)" "$(percentile 0.99)"
  printf '  "requests": [\n'
  for index in "${!labels[@]}"; do
    comma=,
    [[ "$index" -eq $((${#labels[@]} - 1)) ]] && comma=
    printf '    {"name":"%s","status":%s,"latency_seconds":%s}%s\n' \
      "${labels[$index]}" "${statuses[$index]}" "${times[$index]}" "$comma"
  done
  printf '  ]\n}\n'
} > "$results_file"

cat "$results_file"
