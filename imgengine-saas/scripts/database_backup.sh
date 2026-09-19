#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
compose_file="$script_dir/../infra/docker-compose.yml"
backup_dir="${BACKUP_DIR:-$script_dir/../data/backups/postgres}"
postgres_user="${POSTGRES_USER:-imgengine}"
postgres_db="${POSTGRES_DB:-imgengine}"
timestamp="$(date -u +%Y%m%dT%H%M%SZ)"
backup_file="$backup_dir/${postgres_db}-${timestamp}.dump"
container_file="/tmp/imgengine-backup-${timestamp}.dump"

mkdir -p "$backup_dir"
docker compose -f "$compose_file" exec -T db \
  pg_dump --username "$postgres_user" --dbname "$postgres_db" \
  --format=custom --no-owner --no-privileges --file "$container_file"
container_id="$(docker compose -f "$compose_file" ps -q db)"
if [[ -z "$container_id" ]]; then
  echo "Backup failed: PostgreSQL container is not running." >&2
  exit 1
fi

docker compose -f "$compose_file" exec -T db pg_restore --list "$container_file" >/dev/null
docker cp "${container_id}:${container_file}" "$backup_file"
docker compose -f "$compose_file" exec -T db rm -f "$container_file"

if [[ ! -s "$backup_file" ]]; then
  rm -f "$backup_file"
  echo "Backup failed: dump is empty." >&2
  exit 1
fi

echo "Verified PostgreSQL backup: $backup_file"
