#!/usr/bin/env bash
set -euo pipefail

api_url="${API_URL:-http://127.0.0.1:8000}"
api_key="${API_KEYS:?API_KEYS must be set}"
fixture="${1:-imgengine/photo.jpg}"

for attempt in {1..30}; do
  curl --fail --silent "$api_url/healthz" >/dev/null && break
  sleep 1
done

response="$(curl --fail --silent -X POST "$api_url/api/generate" -H "X-API-Key: $api_key" -F "file=@$fixture;type=image/jpeg")"
job_id="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["job_id"])' <<<"$response")"

for attempt in {1..60}; do
  job="$(curl --fail --silent -H "X-API-Key: $api_key" "$api_url/api/status/$job_id")"
  job_status="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["status"])' <<<"$job")"
  if [[ "$job_status" == "completed" ]]; then
    curl --fail --silent -H "X-API-Key: $api_key" "$api_url/api/output/$job_id" --output /tmp/imgengine-output.png
    test -s /tmp/imgengine-output.png
    echo "completed job $job_id"
    exit 0
  fi
  [[ "$job_status" == "failed" ]] && { echo "job $job_id failed: $job" >&2; exit 1; }
  sleep 1
done

echo "job $job_id did not reach a terminal state" >&2
exit 1
