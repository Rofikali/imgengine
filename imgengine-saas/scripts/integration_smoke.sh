#!/usr/bin/env bash
set -euo pipefail

api_url="${API_URL:-http://127.0.0.1:8000}"
api_key="${API_KEY:-${API_KEYS:?API_KEYS must be set}}"
internal_token="${INTERNAL_API_TOKEN:-}"
fixture="${1:-imgengine/photo.jpg}"
content_type="${2:-image/jpeg}"

ready="false"
for attempt in {1..30}; do
  if curl --fail --silent "$api_url/readyz" >/dev/null; then
    ready="true"
    break
  fi
  sleep 1
done
[[ "$ready" == "true" ]] || { echo "API did not become ready" >&2; exit 1; }

idempotency_key="integration-$(basename "$fixture" | tr -cd '[:alnum:]' | cut -c1-80)"
response="$(curl --fail --silent -X POST "$api_url/api/generate" -H "X-API-Key: $api_key" -H "Idempotency-Key: $idempotency_key" -F "file=@$fixture;type=$content_type")"
job_id="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["job_id"])' <<<"$response")"

replay="$(curl --fail --silent -X POST "$api_url/api/generate" -H "X-API-Key: $api_key" -H "Idempotency-Key: $idempotency_key" -F "file=@$fixture;type=$content_type")"
replay_job_id="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["job_id"])' <<<"$replay")"
[[ "$replay_job_id" == "$job_id" ]] || { echo "idempotency replay created a new job" >&2; exit 1; }

for attempt in {1..60}; do
  job="$(curl --fail --silent -H "X-API-Key: $api_key" "$api_url/api/status/$job_id")"
  job_status="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["status"])' <<<"$job")"
  if [[ "$job_status" == "completed" ]]; then
    curl --fail --silent -H "X-API-Key: $api_key" "$api_url/api/output/$job_id" --output /tmp/imgengine-output.jpg
    python3 - <<'PY'
from pathlib import Path

output = Path('/tmp/imgengine-output.jpg').read_bytes()
assert len(output) > 4 and output[:2] == b'\xff\xd8' and output[-2:] == b'\xff\xd9'
PY
    timeline="$(curl --fail --silent -H "X-API-Key: $api_key" "$api_url/api/jobs/$job_id/logs")"
    python3 -c '
import json
import sys
payload = json.load(sys.stdin)
events = [event["event"] for event in payload["events"]]
assert events == ["job_queued", "engine_execution_started", "engine_execution_completed"], events
assert payload["logs"], "missing native job diagnostics"
' <<<"$timeline"
    grep -q "$job_id" ../data/logs/api.log
    grep -q "$job_id" ../data/logs/worker.log
    if [[ -n "$internal_token" ]]; then
      if curl --silent --output /dev/null --write-out '%{http_code}' --request PATCH \
        "$api_url/internal/jobs/$job_id" \
        -H "X-Internal-Token: $internal_token" \
        -H 'Content-Type: application/json' \
        --data '{"status":"processing"}' | grep -qx '409'; then
        :
      else
        echo "completed job $job_id accepted an invalid state transition" >&2
        exit 1
      fi
    fi
    metrics="$(curl --fail --silent "$api_url/metrics")"
    grep -q 'imgengine_job_transitions_total' <<<"$metrics"
    grep -q 'imgengine_upload_bytes' <<<"$metrics"
    echo "completed job $job_id"
    exit 0
  fi
  [[ "$job_status" == "failed" ]] && { echo "job $job_id failed: $job" >&2; exit 1; }
  sleep 1
done

echo "job $job_id did not reach a terminal state" >&2
exit 1
