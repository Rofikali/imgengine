[CmdletBinding()]
param(
  [switch]$Build,
  [string]$ResultsPath = (Join-Path ([System.IO.Path]::GetTempPath()) 'imgengine-p12-local-candidate.json'),
  [int]$StartupTimeoutSeconds = 90
)

$ErrorActionPreference = 'Stop'

if ($StartupTimeoutSeconds -lt 30 -or $StartupTimeoutSeconds -gt 900) {
  throw 'StartupTimeoutSeconds must be between 30 and 900.'
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$shortSha = (& git -C $repoRoot rev-parse --short HEAD).Trim()
$nonce = [Guid]::NewGuid().ToString('N').Substring(0, 12)
$apiImage = "imgengine-local-candidate/rust-api:$shortSha"
$webImage = "imgengine-local-candidate/web:$shortSha"
$smokeImage = 'imgengine-p8-vps-workload:local'
$network = "imgengine-local-candidate-$nonce"
$secretVolume = "imgengine-local-candidate-secrets-$nonce"
$apiContainer = "imgengine-local-candidate-api-$nonce"
$webContainer = "imgengine-local-candidate-web-$nonce"

$keyBytes = New-Object byte[] 32
$rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
$rng.GetBytes($keyBytes)
$rng.Dispose()
$testKey = ([BitConverter]::ToString($keyBytes) -replace '-', '').ToLowerInvariant()

$networkCreated = $false
$volumeCreated = $false
$apiStarted = $false
$webStarted = $false
$secretInit = @'
umask 077
printf '%s\n' "$TEST_KEY" > /secrets/api-keys
printf '%s\n' "$TEST_KEY" > /secrets/render-api-key
chown root:10001 /secrets /secrets/api-keys /secrets/render-api-key
chmod 0710 /secrets
chmod 0440 /secrets/api-keys /secrets/render-api-key
'@
$result = [ordered]@{
  schema = 'imgengine.p12.local-candidate.v1'
  measured_at_utc = [DateTime]::UtcNow.ToString('o')
  git_branch = (& git -C $repoRoot branch --show-current).Trim()
  git_head = (& git -C $repoRoot rev-parse HEAD).Trim()
  docker_version = (& docker version --format '{{.Server.Version}}').Trim()
  compose_version = (& docker compose version --short).Trim()
  scope = 'local Docker Desktop candidate rehearsal; not actual-VPS or production evidence'
  runtime = [ordered]@{
    api_memory_bytes = 268435456
    api_cpus = 0.75
    api_pids_limit = 64
    api_tmpfs_bytes = 100663296
    web_memory_bytes = 201326592
    web_cpus = 0.50
    web_pids_limit = 96
    web_tmpfs_bytes = 16777216
  }
}

function Invoke-Docker([string[]]$Arguments) {
  & docker @Arguments
  if ($LASTEXITCODE -ne 0) {
    # Do not include arguments: an invocation may contain the ephemeral test
    # key while initializing the disposable secret volume.
    throw "Docker command failed with exit code $LASTEXITCODE."
  }
}

function Invoke-Smoke([string]$ScriptPath, [hashtable]$Environment) {
  $arguments = @('run', '--rm', '--network', $network, '--mount', "type=bind,src=$repoRoot,dst=/repo,readonly", '--workdir', '/repo')
  foreach ($name in $Environment.Keys) {
    $arguments += @('--env', "$name=$($Environment[$name])")
  }
  $arguments += @('--entrypoint', 'bash', $smokeImage, $ScriptPath, 'imgengine/photo.jpg')
  Invoke-Docker $arguments
}

try {
  Push-Location $repoRoot

  if ($Build) {
    Invoke-Docker @('build', '--file', 'imgengine/rust/Dockerfile', '--tag', $apiImage, 'imgengine')
    Invoke-Docker @('build', '--file', 'imgengine-saas/frontend/Dockerfile', '--tag', $webImage, 'imgengine-saas/frontend')
  }
  foreach ($image in @($apiImage, $webImage)) {
    & docker image inspect $image *> $null
    if ($LASTEXITCODE -ne 0) {
      throw "Missing $image. Re-run with -Build to create the candidate images from this revision."
    }
  }
  & docker image inspect $smokeImage *> $null
  if ($LASTEXITCODE -ne 0) {
    throw "Missing $smokeImage. Build the reusable local smoke helper first: docker compose -f infra/docker-compose.p8-vps-sim.yml --profile workload build workload"
  }

  Invoke-Docker @('network', 'create', '--internal', $network)
  $networkCreated = $true
  Invoke-Docker @('volume', 'create', $secretVolume)
  $volumeCreated = $true

  # Create a disposable root-owned secret directory. The non-root runtime can
  # traverse/read only through GID 10001; the random test key is never logged
  # or written to the repository.
  Invoke-Docker @(
    'run', '--rm', '--user', '0:0', '--env', "TEST_KEY=$testKey",
    '--mount', "type=volume,src=$secretVolume,dst=/secrets",
    '--entrypoint', '/bin/sh', $apiImage, '-ec',
    $secretInit
  )

  Invoke-Docker @(
    'run', '--detach', '--name', $apiContainer,
    '--network', $network, '--network-alias', 'rust-api',
    '--user', '10001:10001', '--read-only', '--cap-drop', 'ALL',
    '--security-opt', 'no-new-privileges:true', '--memory', '256m',
    '--memory-swap', '256m', '--cpus', '0.75', '--pids-limit', '64',
    '--tmpfs', '/tmp:rw,nosuid,nodev,noexec,mode=1777,size=96m',
    '--env', 'IMGENGINE_API_BIND=0.0.0.0:8000',
    '--env', 'IMGENGINE_API_KEYS_FILE=/run/secrets/api-keys',
    '--env', 'IMGENGINE_WORKSPACE_ROOT=/tmp/imgengine-workspaces',
    '--mount', "type=volume,src=$secretVolume,dst=/run/secrets,readonly",
    $apiImage
  )
  $apiStarted = $true

  Invoke-Docker @(
    'run', '--detach', '--name', $webContainer,
    '--network', $network, '--network-alias', 'web',
    '--user', '10001:10001', '--read-only', '--cap-drop', 'ALL',
    '--security-opt', 'no-new-privileges:true', '--memory', '192m',
    '--memory-swap', '192m', '--cpus', '0.50', '--pids-limit', '96',
    '--tmpfs', '/tmp:rw,nosuid,nodev,noexec,mode=1777,size=16m',
    '--env', 'NUXT_IMGENGINE_API_BASE=http://rust-api:8000',
    '--env', 'IMGENGINE_RENDER_API_KEY_FILE=/run/secrets/render-api-key',
    '--mount', "type=volume,src=$secretVolume,dst=/run/secrets,readonly",
    $webImage
  )
  $webStarted = $true

  $deadline = [DateTime]::UtcNow.AddSeconds($StartupTimeoutSeconds)
  $ready = $false
  do {
    & docker run --rm --network $network --entrypoint curl $smokeImage `
      --fail --silent http://rust-api:8000/readyz *> $null
    if ($LASTEXITCODE -eq 0) {
      $ready = $true
      break
    }
    Start-Sleep -Seconds 1
  } while ([DateTime]::UtcNow -lt $deadline)
  if (-not $ready) {
    throw "Rust API did not become ready within $StartupTimeoutSeconds seconds."
  }

  Invoke-Smoke 'imgengine-saas/scripts/rust_api_smoke.sh' @{ API_URL = 'http://rust-api:8000'; API_KEY = $testKey }
  Invoke-Smoke 'imgengine-saas/scripts/frontend_proxy_smoke.sh' @{ FRONTEND_URL = 'http://web:3000' }

  $workspaceEntries = (& docker exec $apiContainer sh -c 'find /tmp/imgengine-workspaces -mindepth 1 -maxdepth 1 -print | wc -l').Trim()
  if ($LASTEXITCODE -ne 0) { throw 'Unable to inspect the Rust API workspace.' }
  if ($workspaceEntries -ne '0') { throw "Workspace cleanup failed: $workspaceEntries residual entries." }

  $inspect = & docker inspect $apiContainer $webContainer | ConvertFrom-Json
  $result.images = @($inspect | ForEach-Object { [ordered]@{ name=$_.Name; image=$_.Image; image_id=$_.Image; user=$_.Config.User; memory_bytes=$_.HostConfig.Memory; memory_swap_bytes=$_.HostConfig.MemorySwap; nano_cpus=$_.HostConfig.NanoCpus; pids_limit=$_.HostConfig.PidsLimit } })
  $result.smoke = [ordered]@{ api = 'pass'; nuxt_proxy = 'pass'; workspace_entries_after_smoke = [int]$workspaceEntries }
  $result.status = 'pass'
}
catch {
  $result.status = 'fail'
  $result.failure_class = $_.Exception.GetType().FullName
  $result.failure = $_.Exception.Message
  throw
}
finally {
  $result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ResultsPath -Encoding utf8
  if ($webStarted) { & docker rm --force $webContainer 2>$null | Out-Null }
  if ($apiStarted) { & docker rm --force $apiContainer 2>$null | Out-Null }
  if ($networkCreated) { & docker network rm $network 2>$null | Out-Null }
  if ($volumeCreated) { & docker volume rm $secretVolume 2>$null | Out-Null }
  Pop-Location -ErrorAction SilentlyContinue
  Write-Output "P12 local candidate result: $ResultsPath"
}
