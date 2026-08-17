#!/usr/bin/env bash
set -euo pipefail

if [[ "${1:-}" != "--confirm" || -z "${2:-}" || $# -ne 2 ]]; then
  echo "Usage: $0 --confirm /path/to/backup.dump" >&2
  exit 64
fi

backup_file="$2"
if [[ ! -s "$backup_file" ]]; then
  echo "Backup file is missing or empty: $backup_file" >&2
  exit 66
fi

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
compose_file="$script_dir/../infra/docker-compose.yml"
postgres_user="${POSTGRES_USER:-imgengine}"
postgres_db="${POSTGRES_DB:-imgengine}"
container_file="/tmp/imgengine-restore.dump"

container_id="$(docker compose -f "$compose_file" ps -q db)"
if [[ -z "$container_id" ]]; then
  echo "Restore failed: PostgreSQL container is not running." >&2
  exit 1
fi
docker cp "$backup_file" "${container_id}:${container_file}"
docker compose -f "$compose_file" exec -T db pg_restore --list "$container_file" >/dev/null
docker compose -f "$compose_file" stop api worker cleanup
docker compose -f "$compose_file" exec -T db \
  pg_restore --username "$postgres_user" --dbname "$postgres_db" \
  --clean --if-exists --no-owner --no-privileges --exit-on-error "$container_file"
docker compose -f "$compose_file" exec -T db rm -f "$container_file"
docker compose -f "$compose_file" up -d api worker cleanup

echo "Restore completed. Run the smoke test before accepting traffic."
