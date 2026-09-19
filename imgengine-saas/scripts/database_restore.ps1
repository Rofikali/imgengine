param(
    [Parameter(Mandatory = $true)]
    [string]$BackupFile,
    [switch]$Confirm
)

$ErrorActionPreference = "Stop"
if (-not $Confirm) { throw "Restore is destructive. Re-run with -Confirm." }
if (-not (Test-Path -PathType Leaf $BackupFile) -or (Get-Item $BackupFile).Length -eq 0) {
    throw "Backup file is missing or empty: $BackupFile"
}

$scriptDirectory = Split-Path -Parent $PSCommandPath
$composeFile = Join-Path $scriptDirectory "../infra/docker-compose.yml"
$databaseName = if ($env:POSTGRES_DB) { $env:POSTGRES_DB } else { "imgengine" }
$databaseUser = if ($env:POSTGRES_USER) { $env:POSTGRES_USER } else { "imgengine" }
$containerFile = "/tmp/imgengine-restore.dump"
$containerId = (& docker compose -f $composeFile ps -q db).Trim()
if (-not $containerId) { throw "PostgreSQL container is not running." }

& docker cp $BackupFile "${containerId}:$containerFile"
if ($LASTEXITCODE -ne 0) { throw "Could not stage restore file in PostgreSQL container." }
& docker compose -f $composeFile exec -T db pg_restore --list $containerFile
if ($LASTEXITCODE -ne 0) { throw "Backup verification failed." }

& docker compose -f $composeFile stop api worker cleanup
& docker compose -f $composeFile exec -T db pg_restore --username $databaseUser --dbname $databaseName --clean --if-exists --no-owner --no-privileges --exit-on-error $containerFile
if ($LASTEXITCODE -ne 0) { throw "Restore failed. API and worker remain stopped for investigation." }
& docker compose -f $composeFile exec -T db rm -f $containerFile
& docker compose -f $composeFile up -d api worker cleanup

Write-Output "Restore completed. Run the smoke test before accepting traffic."
