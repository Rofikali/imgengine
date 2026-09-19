# IMGENGINE Database Recovery

**Status:** implemented for the single-node Docker deployment. PostgreSQL state is held in the named `postgres-data` volume; it survives container recreation but not `docker compose down -v`.

## Backup

Run a verified, consistent PostgreSQL custom-format dump while the database is healthy:

```powershell
./imgengine-saas/scripts/database_backup.ps1
```

```bash
./imgengine-saas/scripts/database_backup.sh
```

Backups are written outside the database container at `imgengine-saas/data/backups/postgres/` and ignored by Git. Each script rejects an empty dump and uses `pg_restore --list` to validate it before reporting success. Set `BACKUP_DIR` (Bash) or `-BackupDirectory` (PowerShell) to write to a mounted, encrypted backup destination.

## Restore

Restoration replaces the current job metadata and audit timeline. Preserve the current state with a fresh backup first, verify the requested dump, and perform it during a maintenance window. The scripts stop API, worker, and cleanup services during the destructive operation and start them only after PostgreSQL reports success.

```powershell
./imgengine-saas/scripts/database_restore.ps1 -BackupFile ./imgengine-saas/data/backups/postgres/imgengine-YYYYMMDDTHHMMSSZ.dump -Confirm
```

```bash
./imgengine-saas/scripts/database_restore.sh --confirm ./imgengine-saas/data/backups/postgres/imgengine-YYYYMMDDTHHMMSSZ.dump
```

Run `./imgengine-saas/scripts/verify.ps1` (or `integration_smoke.sh` in Linux CI) after a restore and before reopening traffic.

## Operational Policy

1. Schedule daily backups and retain them according to the product retention policy; replicate them to encrypted storage outside this host.
2. Perform and record a restore drill at least quarterly. A successful backup without a restore test is not recovery evidence.
3. Backup local artifacts under `imgengine-saas/data/uploads` and `imgengine-saas/data/outputs` on the same schedule. The database does not contain customer image bytes.
4. Before multi-node production deployment, replace this single-node volume process with managed PostgreSQL automated backups, point-in-time recovery, cross-zone replication, and tested runbooks.
