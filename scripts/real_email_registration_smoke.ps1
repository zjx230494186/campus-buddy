param(
    [string]$EnvFile = "D:\big_homework_private\smtp.env",
    [string]$BaseUrl = "http://localhost:8080/api",
    [switch]$StartBackend,
    [switch]$SendOnly,
    [switch]$CompleteRegistration
)

$ErrorActionPreference = "Stop"

function Load-EnvFile {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Private env file not found: $Path"
    }

    foreach ($line in Get-Content -LiteralPath $Path -Encoding UTF8) {
        $trimmed = $line.Trim()
        if ($trimmed.Length -eq 0 -or $trimmed.StartsWith("#")) {
            continue
        }
        $parts = $trimmed.Split("=", 2)
        if ($parts.Count -ne 2) {
            continue
        }
        $name = $parts[0].Trim()
        $value = $parts[1].Trim()
        if ($value.StartsWith('"') -and $value.EndsWith('"') -and $value.Length -ge 2) {
            $value = $value.Substring(1, $value.Length - 2)
        }
        [Environment]::SetEnvironmentVariable($name, $value, "Process")
    }
}

function Get-EnvStatus {
    param([string[]]$Names)

    foreach ($name in $Names) {
        $value = [Environment]::GetEnvironmentVariable($name, "Process")
        $status = if ([string]::IsNullOrWhiteSpace($value)) { "missing" } else { "present" }
        Write-Host "$name=$status"
    }
}

function Assert-EnvPresent {
    param([string[]]$Names)

    $missing = @()
    foreach ($name in $Names) {
        $value = [Environment]::GetEnvironmentVariable($name, "Process")
        if ([string]::IsNullOrWhiteSpace($value)) {
            $missing += $name
        }
    }
    if ($missing.Count -gt 0) {
        throw "Missing required private env variables: $($missing -join ', ')"
    }
}

function Invoke-JsonPost {
    param(
        [string]$Url,
        [object]$Body
    )

    $json = $Body | ConvertTo-Json -Depth 8
    try {
        return Invoke-RestMethod -Method Post -Uri $Url -ContentType "application/json; charset=utf-8" -Body $json
    } catch {
        $response = $_.Exception.Response
        if ($response -and $response.GetResponseStream()) {
            $reader = [System.IO.StreamReader]::new($response.GetResponseStream())
            $bodyText = $reader.ReadToEnd()
            throw "POST failed: $Url`n$bodyText"
        }
        throw
    }
}

function Test-BackendHealth {
    param([string]$Url)

    try {
        $health = Invoke-RestMethod -Method Get -Uri "$Url/health" -TimeoutSec 3
        return $health.status -eq "UP"
    } catch {
        return $false
    }
}

function Start-LocalBackend {
    param(
        [string]$RepoRoot,
        [string]$PrivateDir,
        [string]$Url
    )

    if (Test-BackendHealth $Url) {
        Write-Host "backend=already-running"
        return
    }

    if (-not $StartBackend) {
        throw "Backend is not running. Re-run with -StartBackend or start it manually."
    }

    New-Item -ItemType Directory -Force -Path $PrivateDir | Out-Null
    $stdout = Join-Path $PrivateDir "backend-smtp-smoke.out.log"
    $stderr = Join-Path $PrivateDir "backend-smtp-smoke.err.log"
    $backendDir = Join-Path $RepoRoot "backend"
    $mvnw = Join-Path $backendDir "mvnw.cmd"

    $process = Start-Process -FilePath $mvnw `
        -ArgumentList @("spring-boot:run", "-Dspring-boot.run.profiles=local-h2") `
        -WorkingDirectory $backendDir `
        -RedirectStandardOutput $stdout `
        -RedirectStandardError $stderr `
        -WindowStyle Hidden `
        -PassThru

    Write-Host "backend=starting pid=$($process.Id)"
    for ($i = 0; $i -lt 60; $i++) {
        Start-Sleep -Seconds 2
        if (Test-BackendHealth $Url) {
            Write-Host "backend=ready"
            return
        }
        if ($process.HasExited) {
            throw "Backend exited before becoming healthy. Check private logs in $PrivateDir"
        }
    }
    throw "Backend did not become healthy within timeout. Check private logs in $PrivateDir"
}

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$privateDir = Split-Path -Parent $EnvFile
Load-EnvFile $EnvFile

$smtpVars = @(
    "CAMPUS_EMAIL_DELIVERY_MODE",
    "CAMPUS_EMAIL_ALLOWED_DOMAIN",
    "CAMPUS_EMAIL_SMTP_HOST",
    "CAMPUS_EMAIL_SMTP_PORT",
    "CAMPUS_EMAIL_SMTP_USERNAME",
    "CAMPUS_EMAIL_SMTP_PASSWORD",
    "CAMPUS_EMAIL_SMTP_FROM",
    "CAMPUS_EMAIL_SMTP_FROM_NAME",
    "CAMPUS_EMAIL_SMTP_AUTH",
    "CAMPUS_EMAIL_SMTP_START_TLS",
    "CAMPUS_EMAIL_SMTP_SSL"
)
$registrationVars = @(
    "CAMPUS_BUDDY_REAL_REGISTER_EMAIL",
    "CAMPUS_BUDDY_REAL_REGISTER_PASSWORD",
    "CAMPUS_BUDDY_REAL_REGISTER_DISPLAY_NAME"
)

Write-Host "private_env=$EnvFile"
Get-EnvStatus ($smtpVars + $registrationVars + @("CAMPUS_BUDDY_REAL_REGISTER_CODE"))
Assert-EnvPresent ($smtpVars + $registrationVars)

if ($env:CAMPUS_EMAIL_DELIVERY_MODE -ne "smtp") {
    throw "CAMPUS_EMAIL_DELIVERY_MODE must be smtp for real email smoke."
}

$email = $env:CAMPUS_BUDDY_REAL_REGISTER_EMAIL
$password = $env:CAMPUS_BUDDY_REAL_REGISTER_PASSWORD
$displayName = $env:CAMPUS_BUDDY_REAL_REGISTER_DISPLAY_NAME

Start-LocalBackend -RepoRoot $repoRoot -PrivateDir $privateDir -Url $BaseUrl

if (-not $CompleteRegistration) {
    Invoke-JsonPost "$BaseUrl/auth/campus-email/verification-codes" @{
        campusEmail = $email
        purpose = "REGISTER_OR_LOGIN"
    } | Out-Null

    Write-Host "send_code=ok"
    Write-Host "next=put received code into CAMPUS_BUDDY_REAL_REGISTER_CODE in the private env file, then rerun with -CompleteRegistration"
    if ($SendOnly) {
        exit 0
    }
    exit 0
}

Assert-EnvPresent @("CAMPUS_BUDDY_REAL_REGISTER_CODE")
$code = $env:CAMPUS_BUDDY_REAL_REGISTER_CODE

$verification = Invoke-JsonPost "$BaseUrl/auth/campus-email/verifications" @{
    campusEmail = $email
    code = $code
    purpose = "REGISTER_OR_LOGIN"
}

if ([string]::IsNullOrWhiteSpace($verification.verificationTicket)) {
    throw "Verification did not return a verification ticket."
}

Invoke-JsonPost "$BaseUrl/auth/register" @{
    campusEmail = $email
    verificationTicket = $verification.verificationTicket
    password = $password
    displayName = $displayName
} | Out-Null

$login = Invoke-JsonPost "$BaseUrl/auth/login" @{
    campusEmail = $email
    password = $password
}

if ([string]::IsNullOrWhiteSpace($login.accessToken)) {
    throw "Login after registration did not return an access token."
}

Write-Host "verify_code=ok"
Write-Host "register=ok"
Write-Host "login=ok"
