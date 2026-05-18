param(
    [string] $ComposeFile = "$PSScriptRoot\docker-compose.yml",
    [string] $HealthUrl = "http://localhost:8080/api/health",
    [int] $TimeoutSeconds = 90
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $ComposeFile)) {
    Write-Error "Compose file not found: $ComposeFile"
}

$projectName = "campus-buddy-local"

try {
    docker compose -p $projectName -f $ComposeFile up -d --build

    $deadline = (Get-Date).AddSeconds($TimeoutSeconds)
    do {
        try {
            $rawResponse = & curl.exe --fail --silent --max-time 5 $HealthUrl
            $response = $rawResponse | ConvertFrom-Json
            if ($response.status -eq "UP") {
                Write-Host "Health check passed: $HealthUrl returned status=UP"
                exit 0
            }

            Write-Host "Health endpoint responded, but status was not UP."
        } catch {
            Write-Host "Waiting for backend health endpoint..."
        }

        Start-Sleep -Seconds 3
    } while ((Get-Date) -lt $deadline)

    Write-Error "Health check did not pass within $TimeoutSeconds seconds: $HealthUrl"
} finally {
    docker compose -p $projectName -f $ComposeFile down --remove-orphans
}
