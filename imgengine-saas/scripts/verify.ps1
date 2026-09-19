param(
    [string]$ApiKey = "local-test-key",
    [string]$InternalToken = "local-internal-token"
)

$ErrorActionPreference = "Stop"
$env:API_KEYS = $ApiKey
$env:WEB_API_KEY = $ApiKey
$env:INTERNAL_API_TOKEN = $InternalToken

Push-Location "$PSScriptRoot\..\infra"
try {
    docker compose up --build -d api worker web
    $env:API_URL = "http://127.0.0.1:8000"
    $env:API_KEY = $ApiKey
    $env:API_KEYS = $ApiKey
    bash ../scripts/integration_smoke.sh ../../imgengine/photo.jpg image/jpeg
}
finally {
    Pop-Location
}
