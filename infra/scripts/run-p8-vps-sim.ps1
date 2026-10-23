[CmdletBinding()]
param(
  [string]$ResultsPath = (Join-Path ([System.IO.Path]::GetTempPath()) 'imgengine-p8-vps-sim-results.json'),
  [switch]$Build,
  [ValidateRange(30, 900)]
  [int]$StartupTimeoutSeconds = 600
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$simCompose = 'infra/docker-compose.p8-vps-sim.yml'
$project = 'imgengine-p8-vps-sim'
$apiKeyBytes = New-Object byte[] 32
$rng = [System.Security.Cryptography.RandomNumberGenerator]::Create()
$rng.GetBytes($apiKeyBytes)
$rng.Dispose()
$originalKey = $env:IMGENGINE_VPS_SIM_API_KEY
$env:IMGENGINE_VPS_SIM_API_KEY = ([BitConverter]::ToString($apiKeyBytes) -replace '-', '').ToLowerInvariant()
$started = $false
$statsJob = $null
$statsPath = Join-Path ([System.IO.Path]::GetTempPath()) 'imgengine-p8-vps-sim-stats.csv'

function Invoke-Compose([string[]]$Arguments) {
  & docker compose --project-name $project -f $simCompose @Arguments
  if ($LASTEXITCODE -ne 0) {
    throw "docker compose $($Arguments -join ' ') failed with exit code $LASTEXITCODE."
  }
}

try {
  Push-Location $repoRoot
  if ($Build) { Invoke-Compose @('--profile', 'workload', 'build') }
  $startupStopwatch = [System.Diagnostics.Stopwatch]::StartNew()
  Invoke-Compose @('up', '-d', '--no-build')
  $started = $true
  for ($attempt = 0; $attempt -lt $StartupTimeoutSeconds; $attempt++) {
    $frontend = & docker compose --project-name $project -f $simCompose ps -q frontend
    $api = & docker compose --project-name $project -f $simCompose ps -q rust-api
    if ($frontend -and $api) {
      $frontendHealth = docker inspect --format '{{.State.Health.Status}}' $frontend 2>$null
      $apiHealth = docker inspect --format '{{.State.Health.Status}}' $api 2>$null
      if ($frontendHealth -eq 'healthy' -and $apiHealth -eq 'healthy') { break }
    }
    Start-Sleep -Seconds 1
  }
  if ($frontendHealth -ne 'healthy' -or $apiHealth -ne 'healthy') {
    throw "VPS-sim services did not become healthy within $StartupTimeoutSeconds seconds (frontend=$frontendHealth rust-api=$apiHealth)."
  }
  $startupStopwatch.Stop()

  Remove-Item -LiteralPath $statsPath -Force -ErrorAction SilentlyContinue
  $statsJob = Start-Job -ArgumentList $frontend,$api,$statsPath -ScriptBlock {
    param($frontendId, $apiId, $output)
    while ($true) {
      $timestamp = [DateTime]::UtcNow.ToString('o')
      $rows = & docker stats --no-stream --format '{{.Container}},{{.CPUPerc}},{{.MemUsage}},{{.PIDs}}' $frontendId $apiId 2>$null
      foreach ($row in $rows) { "$timestamp,$row" | Add-Content -LiteralPath $output }
      Start-Sleep -Milliseconds 500
    }
  }

  $workloadJson = & docker compose --project-name $project -f $simCompose --profile workload run --rm -T workload /tmp/workload.json
  if ($LASTEXITCODE -ne 0) {
    $workloadJson | Out-Host
    throw "P8 VPS workload failed with exit code $LASTEXITCODE."
  }

  # The request lifecycle owns per-request directories.  The constrained
  # runtime mounts the root as a throw-away tmpfs; checking it while the API is
  # still live verifies normal request cleanup rather than inferring it from
  # container removal below.
  $workspaceEntries = (& docker compose --project-name $project -f $simCompose exec -T rust-api sh -c 'find /tmp/imgengine-workspaces -mindepth 1 -maxdepth 1 -print | wc -l').Trim()
  if ($LASTEXITCODE -ne 0) { throw 'Unable to inspect the API workspace after the workload.' }
  if ($workspaceEntries -ne '0') { throw "Request workspace cleanup failed: $workspaceEntries residual entries." }

  if ($statsJob) {
    Stop-Job $statsJob -ErrorAction SilentlyContinue
    Receive-Job $statsJob -ErrorAction SilentlyContinue | Out-Null
    Remove-Job $statsJob -Force -ErrorAction SilentlyContinue
    $statsJob = $null
  }
  $statsSampleCount = if (Test-Path -LiteralPath $statsPath) { @(Get-Content -LiteralPath $statsPath).Count } else { 0 }

  $inspect = & docker inspect $frontend $api | ConvertFrom-Json
  $result = [ordered]@{
    schema = 'imgengine.p8.vps-sim.v1'
    measured_at_utc = [DateTime]::UtcNow.ToString('o')
    git_branch = (& git branch --show-current).Trim()
    git_head = (& git rev-parse HEAD).Trim()
    docker_version = (& docker version --format '{{.Server.Version}}').Trim()
    compose_version = (& docker compose version --short).Trim()
    startup_seconds = [Math]::Round($startupStopwatch.Elapsed.TotalSeconds, 3)
    containers = @($inspect | ForEach-Object {
      [ordered]@{ name=$_.Name; image=$_.Image; memory_bytes=$_.HostConfig.Memory; memory_swap_bytes=$_.HostConfig.MemorySwap; nano_cpus=$_.HostConfig.NanoCpus; pids_limit=$_.HostConfig.PidsLimit; tmpfs=$_.HostConfig.Tmpfs }
    })
    workload = ($workloadJson | ConvertFrom-Json)
    cleanup = [ordered]@{ workspace_entries_after_workload=[int]$workspaceEntries; status='pass' }
    docker_stats_sampling = [ordered]@{ interval_ms=500; samples=$statsSampleCount; path=$statsPath; limitation='periodic docker stats samples are not exact memory peaks' }
    limitation = 'Docker Desktop Linux VM has its own 4 GiB allocation and cgroup v1. Service limits do not prove total 1 GiB VPS behavior.'
  }
  $result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ResultsPath -Encoding utf8
  Write-Output "P8 VPS simulation results: $ResultsPath"
}
finally {
  if ($statsJob) { Stop-Job $statsJob -ErrorAction SilentlyContinue; Remove-Job $statsJob -Force -ErrorAction SilentlyContinue }
  if ($started) { & docker compose --project-name $project -f $simCompose down --remove-orphans | Out-Host }
  if ($null -eq $originalKey) { Remove-Item Env:IMGENGINE_VPS_SIM_API_KEY -ErrorAction SilentlyContinue } else { $env:IMGENGINE_VPS_SIM_API_KEY = $originalKey }
  Pop-Location -ErrorAction SilentlyContinue
}
