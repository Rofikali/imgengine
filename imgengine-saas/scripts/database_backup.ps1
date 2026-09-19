param(
    [string]$BackupDirectory = ""
)

$ErrorActionPreference = "Stop"
$scriptDirectory = Split-Path -Parent $PSCommandPath
$composeFile = Join-Path $scriptDirectory "../infra/docker-compose.yml"
if (-not $BackupDirectory) {
    $BackupDirectory = Join-Path $scriptDirectory "../data/backups/postgres"
}

$databaseName = if ($env:POSTGRES_DB) { $env:POSTGRES_DB } else { "imgengine" }
$databaseUser = if ($env:POSTGRES_USER) { $env:POSTGRES_USER } else { "imgengine" }
$timestamp = (Get-Date).ToUniversalTime().ToString("yyyyMMddTHHmmssZ")
$backupFile = Join-Path $BackupDirectory "$databaseName-$timestamp.dump"
$containerFile = "/tmp/imgengine-backup-$timestamp.dump"

New-Item -ItemType Directory -Force -Path $BackupDirectory | Out-Null
& docker compose -f $composeFile exec -T db pg_dump --username $databaseUser --dbname $databaseName --format=custom --no-owner --no-privileges --file $containerFile
if ($LASTEXITCODE -ne 0) { throw "PostgreSQL dump failed." }
$containerId = (& docker compose -f $composeFile ps -q db).Trim()
if (-not $containerId) { throw "PostgreSQL container is not running." }

& docker compose -f $composeFile exec -T db pg_restore --list $containerFile
if ($LASTEXITCODE -ne 0) { throw "Backup verification failed." }
& docker cp "${containerId}:$containerFile" $backupFile
if ($LASTEXITCODE -ne 0 -or (Get-Item $backupFile).Length -eq 0) { throw "Backup copy failed or was empty." }
& docker compose -f $composeFile exec -T db rm -f $containerFile

Write-Output "Verified PostgreSQL backup: $backupFile"
