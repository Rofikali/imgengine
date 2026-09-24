[CmdletBinding()]
param(
  [string]$EnvironmentFile = 'infra/p7-frontend-dev.env'
)

$ErrorActionPreference = 'Stop'
$composeFile = 'infra/docker-compose.p7-frontend-dev.yml'
if (-not (Test-Path -LiteralPath $EnvironmentFile)) {
  throw "Missing $EnvironmentFile. Copy infra/p7-frontend-dev.env.example and set a local test-only key."
}

$compose = @('compose', '--env-file', $EnvironmentFile, '-f', $composeFile)
function Invoke-Compose([string[]]$Arguments) { & docker @compose @Arguments }

try {
  Invoke-Compose @('up', '--build', '-d')
  $ready = $false
  foreach ($attempt in 1..90) {
    try {
      Invoke-WebRequest -UseBasicParsing http://127.0.0.1:3000/ | Out-Null
      $ready = $true
      break
    } catch { Start-Sleep -Seconds 1 }
  }
  if (-not $ready) { throw 'Nuxt did not become reachable on http://127.0.0.1:3000.' }

  Invoke-Compose @('exec', '-T', 'frontend', 'bash', '/workspace/imgengine/infra/scripts/verify-p7-render-contract.sh')

  Invoke-Compose @('stop', 'rust-api')
  try {
    Invoke-Compose @('exec', '-T', 'frontend', 'bash', '-lc', @'
head -c 1100000 /dev/zero > /tmp/imgengine-p7-oversized
status=$(curl --silent --output /dev/null --write-out "%{http_code}" -F file=@/tmp/imgengine-p7-oversized http://localhost:3000/api/render)
rm -f /tmp/imgengine-p7-oversized
test "$status" = 413
'@)
    $status = & curl.exe --silent --output NUL --write-out '%{http_code}' `
      -F 'file=@imgengine/tests/fixtures/cc0_camera_landscape.jpg;type=image/jpeg' `
      http://127.0.0.1:3000/api/render
    if ($status -ne '503') { throw "Expected 503 when Rust is unavailable; received $status." }
  } finally { Invoke-Compose @('start', 'rust-api') }

  $keyLine = Get-Content -LiteralPath $EnvironmentFile | Where-Object { $_ -match '^IMGENGINE_DEV_API_KEY=' } | Select-Object -First 1
  $key = $keyLine.Substring('IMGENGINE_DEV_API_KEY='.Length)
  & docker @compose exec -T frontend sh -c 'grep -Rqs -- "$1" .nuxt/dist/client' sh $key
  if ($LASTEXITCODE -eq 0) { throw 'Development API key found in Nuxt client bundle.' }
  if ($LASTEXITCODE -gt 1) { throw 'Unable to inspect Nuxt client bundle for the development key.' }

  Write-Output 'P7 frontend stack verification passed'
} finally {
  Invoke-Compose @('down', '--remove-orphans')
}
