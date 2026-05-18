param(
    [string]$Endpoint = "obs.cn-north-4.myhuaweicloud.com",
    [string]$Region = "cn-north-4",
    [string]$Bucket = "20260518-bighomework",
    [string]$KeyPrefix = "technical-spike/min-connectivity"
)

$ErrorActionPreference = "Stop"

function Get-FirstEnvValue {
    param([string[]]$Names)

    foreach ($name in $Names) {
        $value = [Environment]::GetEnvironmentVariable($name)
        if (-not [string]::IsNullOrWhiteSpace($value)) {
            return @{
                Name = $name
                Value = $value
            }
        }
    }

    return $null
}

function ConvertTo-Hex {
    param([byte[]]$Bytes)

    return (($Bytes | ForEach-Object { $_.ToString("x2") }) -join "")
}

function Get-Sha256Hex {
    param([byte[]]$Bytes)

    $sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        return ConvertTo-Hex -Bytes ($sha256.ComputeHash($Bytes))
    }
    finally {
        $sha256.Dispose()
    }
}

function Get-HmacSha256 {
    param(
        [byte[]]$Key,
        [string]$Message
    )

    $hmac = [System.Security.Cryptography.HMACSHA256]::new($Key)
    try {
        return $hmac.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($Message))
    }
    finally {
        $hmac.Dispose()
    }
}

function Get-SignatureKey {
    param(
        [string]$SecretKey,
        [string]$DateStamp,
        [string]$RegionName,
        [string]$ServiceName
    )

    $dateKey = Get-HmacSha256 -Key ([System.Text.Encoding]::UTF8.GetBytes("AWS4$SecretKey")) -Message $DateStamp
    $dateRegionKey = Get-HmacSha256 -Key $dateKey -Message $RegionName
    $dateRegionServiceKey = Get-HmacSha256 -Key $dateRegionKey -Message $ServiceName
    return Get-HmacSha256 -Key $dateRegionServiceKey -Message "aws4_request"
}

function Invoke-ObsSignedRequest {
    param(
        [string]$Method,
        [string]$Uri,
        [byte[]]$BodyBytes,
        [string]$AccessKeyId,
        [string]$SecretAccessKey,
        [string]$SecurityToken
    )

    $service = "s3"
    $hostName = "$Bucket.$Endpoint"
    $now = [DateTime]::UtcNow
    $amzDate = $now.ToString("yyyyMMddTHHmmssZ", [Globalization.CultureInfo]::InvariantCulture)
    $dateStamp = $now.ToString("yyyyMMdd", [Globalization.CultureInfo]::InvariantCulture)
    $payloadHash = Get-Sha256Hex -Bytes $BodyBytes

    $canonicalHeaders = "host:$hostName`n" +
        "x-amz-content-sha256:$payloadHash`n" +
        "x-amz-date:$amzDate`n"
    $signedHeaders = "host;x-amz-content-sha256;x-amz-date"

    if (-not [string]::IsNullOrWhiteSpace($SecurityToken)) {
        $canonicalHeaders += "x-amz-security-token:$SecurityToken`n"
        $signedHeaders += ";x-amz-security-token"
    }

    $canonicalRequest = @(
        $Method
        $Uri
        ""
        $canonicalHeaders
        $signedHeaders
        $payloadHash
    ) -join "`n"

    $credentialScope = "$dateStamp/$Region/$service/aws4_request"
    $stringToSign = @(
        "AWS4-HMAC-SHA256"
        $amzDate
        $credentialScope
        (Get-Sha256Hex -Bytes ([System.Text.Encoding]::UTF8.GetBytes($canonicalRequest)))
    ) -join "`n"

    $signingKey = Get-SignatureKey -SecretKey $SecretAccessKey -DateStamp $dateStamp -RegionName $Region -ServiceName $service
    $signature = ConvertTo-Hex -Bytes (Get-HmacSha256 -Key $signingKey -Message $stringToSign)
    $authorizationHeader = "AWS4-HMAC-SHA256 Credential=$AccessKeyId/$credentialScope, SignedHeaders=$signedHeaders, Signature=$signature"

    $headers = @{
        "Authorization" = $authorizationHeader
        "x-amz-content-sha256" = $payloadHash
        "x-amz-date" = $amzDate
    }

    if (-not [string]::IsNullOrWhiteSpace($SecurityToken)) {
        $headers["x-amz-security-token"] = $SecurityToken
    }

    $requestUri = "https://$hostName$Uri"
    $requestArgs = @{
        Method = $Method
        Uri = $requestUri
        Headers = $headers
        UseBasicParsing = $true
    }

    if ($BodyBytes.Length -gt 0) {
        $requestArgs["Body"] = $BodyBytes
        $requestArgs["ContentType"] = "text/plain; charset=utf-8"
    }

    return Invoke-WebRequest @requestArgs
}

$accessKey = Get-FirstEnvValue -Names @(
    "OBJECT_STORAGE_ACCESS_KEY_ID",
    "OBS_ACCESS_KEY_ID",
    "HUAWEICLOUD_OBS_ACCESS_KEY_ID",
    "AWS_ACCESS_KEY_ID"
)
$secretKey = Get-FirstEnvValue -Names @(
    "OBJECT_STORAGE_SECRET_ACCESS_KEY",
    "OBS_SECRET_ACCESS_KEY",
    "HUAWEICLOUD_OBS_SECRET_ACCESS_KEY",
    "AWS_SECRET_ACCESS_KEY"
)
$securityToken = Get-FirstEnvValue -Names @(
    "OBJECT_STORAGE_SESSION_TOKEN",
    "OBS_SESSION_TOKEN",
    "HUAWEICLOUD_OBS_SESSION_TOKEN",
    "AWS_SESSION_TOKEN"
)

if ($null -eq $accessKey -or $null -eq $secretKey) {
    Write-Host "OBS minimal connectivity probe skipped: missing server-side credentials in environment."
    Write-Host "Accepted access key env names: OBJECT_STORAGE_ACCESS_KEY_ID, OBS_ACCESS_KEY_ID, HUAWEICLOUD_OBS_ACCESS_KEY_ID, AWS_ACCESS_KEY_ID"
    Write-Host "Accepted secret key env names: OBJECT_STORAGE_SECRET_ACCESS_KEY, OBS_SECRET_ACCESS_KEY, HUAWEICLOUD_OBS_SECRET_ACCESS_KEY, AWS_SECRET_ACCESS_KEY"
    Write-Host "Optional session token env names: OBJECT_STORAGE_SESSION_TOKEN, OBS_SESSION_TOKEN, HUAWEICLOUD_OBS_SESSION_TOKEN, AWS_SESSION_TOKEN"
    exit 2
}

$objectKey = "$KeyPrefix/$([Guid]::NewGuid().ToString("N")).txt"
$objectUri = "/" + ($objectKey -replace " ", "%20")
$payloadText = "campus-buddy OBS minimal connectivity probe`ncreatedUtc=$([DateTime]::UtcNow.ToString("o", [Globalization.CultureInfo]::InvariantCulture))`n"
$payloadBytes = [System.Text.Encoding]::UTF8.GetBytes($payloadText)
$expectedHash = Get-Sha256Hex -Bytes $payloadBytes

Write-Host "OBS minimal connectivity probe started."
Write-Host "Endpoint: $Endpoint"
Write-Host "Region: $Region"
Write-Host "Bucket: $Bucket"
Write-Host "Credential env names: $($accessKey.Name), $($secretKey.Name)$(if ($securityToken) { ', ' + $securityToken.Name } else { '' })"
Write-Host "Object key: $objectKey"

try {
    $putResponse = Invoke-ObsSignedRequest -Method "PUT" -Uri $objectUri -BodyBytes $payloadBytes -AccessKeyId $accessKey.Value -SecretAccessKey $secretKey.Value -SecurityToken $securityToken.Value
    Write-Host "PUT status: $([int]$putResponse.StatusCode)"

    $getResponse = Invoke-ObsSignedRequest -Method "GET" -Uri $objectUri -BodyBytes ([byte[]]::new(0)) -AccessKeyId $accessKey.Value -SecretAccessKey $secretKey.Value -SecurityToken $securityToken.Value
    Write-Host "GET status: $([int]$getResponse.StatusCode)"

    $downloadedBytes = [System.Text.Encoding]::UTF8.GetBytes($getResponse.Content)
    $actualHash = Get-Sha256Hex -Bytes $downloadedBytes
    if ($actualHash -ne $expectedHash) {
        throw "Downloaded object hash mismatch. Expected $expectedHash, got $actualHash."
    }
    Write-Host "SHA-256 verified: $actualHash"

    $deleteResponse = Invoke-ObsSignedRequest -Method "DELETE" -Uri $objectUri -BodyBytes ([byte[]]::new(0)) -AccessKeyId $accessKey.Value -SecretAccessKey $secretKey.Value -SecurityToken $securityToken.Value
    Write-Host "DELETE status: $([int]$deleteResponse.StatusCode)"

    Write-Host "OBS minimal connectivity probe passed."
}
catch {
    Write-Host "OBS minimal connectivity probe failed."
    Write-Host $_.Exception.Message
    exit 1
}
