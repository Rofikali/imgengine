#!/usr/bin/env bash
set -euo pipefail

compose_file=infra/docker-compose.p7-frontend-dev.yml
env_file=infra/p7-frontend-dev.env

if [[ ! -f "$env_file" ]]; then
  echo "missing $env_file; copy $env_file.example and set a local test-only key" >&2
  exit 2
fi

compose=(docker compose --env-file "$env_file" -f "$compose_file")
cleanup() { "${compose[@]}" down --remove-orphans; }
trap cleanup EXIT

"${compose[@]}" up --build -d

for _ in $(seq 1 90); do
  if curl --silent --show-error --fail http://127.0.0.1:3000/ >/dev/null; then break; fi
  sleep 1
done
curl --silent --show-error --fail http://127.0.0.1:3000/ >/dev/null

"${compose[@]}" exec -T frontend bash /workspace/imgengine/infra/scripts/verify-p7-render-contract.sh
"${compose[@]}" exec -T frontend bash /workspace/imgengine/infra/scripts/verify-p8-layout-contract.sh

"${compose[@]}" stop rust-api
"${compose[@]}" exec -T frontend bash -lc '
  head -c 1100000 /dev/zero > /tmp/imgengine-p7-oversized
  status=$(curl --silent --output /tmp/imgengine-p7-problem --write-out "%{http_code}" \
    -F file=@/tmp/imgengine-p7-oversized http://localhost:3000/api/render)
  rm -f /tmp/imgengine-p7-oversized
  [[ "$status" == 413 ]]
  grep -q '"code":"resource_limit"' /tmp/imgengine-p7-problem
  ! grep -q '"stack"' /tmp/imgengine-p7-problem
  rm -f /tmp/imgengine-p7-problem
'
[[ $(curl --silent --output /dev/null --write-out '%{http_code}' \
  -F "file=@imgengine/tests/fixtures/cc0_camera_landscape.jpg;type=image/jpeg" \
  http://127.0.0.1:3000/api/render) == 503 ]]
"${compose[@]}" start rust-api

# Private Nuxt runtime values are server-only. Do not print the key even when
# this assertion fails.
key=$(sed -n 's/^IMGENGINE_DEV_API_KEY=//p' "$env_file")
if "${compose[@]}" exec -T frontend sh -c 'grep -Rqs -- "$1" .nuxt/dist/client' sh "$key"; then
  echo "development API key found in client bundle" >&2
  exit 1
fi

echo 'P7 frontend stack verification passed'
