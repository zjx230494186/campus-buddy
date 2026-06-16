param(
    [string]$BaseUrl = $env:CAMPUS_BUDDY_API_BASE_URL,
    [switch]$AllowIdentityReviewMutation
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($BaseUrl)) {
    $BaseUrl = "http://114.116.203.78/api"
}
$BaseUrl = $BaseUrl.TrimEnd("/")

$RequiredEnv = @(
    "CAMPUS_BUDDY_SMOKE_EMAIL",
    "CAMPUS_BUDDY_SMOKE_PASSWORD",
    "CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL",
    "CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD"
)

foreach ($name in $RequiredEnv) {
    if ([string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable($name, "Process"))) {
        throw "Missing required environment variable: $name"
    }
}

$script:Results = New-Object System.Collections.Generic.List[object]
$script:Suffix = (Get-Date -Format "MMddHHmmss")

function Add-Result {
    param(
        [string]$Id,
        [string]$Name,
        [string]$Status,
        [string]$Detail = ""
    )

    $script:Results.Add([pscustomobject]@{
        Id = $Id
        Name = $Name
        Status = $Status
        Detail = $Detail
    }) | Out-Null

    $line = "{0} {1} - {2}" -f $Status.PadRight(4), $Id, $Name
    if (-not [string]::IsNullOrWhiteSpace($Detail)) {
        $line = "$line :: $Detail"
    }
    Write-Host $line
}

function Convert-JsonSafe {
    param([string]$Text)
    if ([string]::IsNullOrWhiteSpace($Text)) {
        return $null
    }
    try {
        return $Text | ConvertFrom-Json
    } catch {
        return $null
    }
}

function Read-ErrorBody {
    param($Response)

    if ($null -eq $Response) {
        return ""
    }

    try {
        $stream = $Response.GetResponseStream()
        if ($stream) {
            $reader = New-Object System.IO.StreamReader($stream)
            return $reader.ReadToEnd()
        }
    } catch {
        return ""
    }
    return ""
}

function Invoke-Api {
    param(
        [string]$Method,
        [string]$Path,
        [string]$Token = "",
        [object]$Body = $null,
        [hashtable]$Headers = @{}
    )

    $uri = if ($Path.StartsWith("http")) { $Path } else { "$BaseUrl$Path" }
    $requestHeaders = @{}
    foreach ($key in $Headers.Keys) {
        $requestHeaders[$key] = $Headers[$key]
    }
    if (-not [string]::IsNullOrWhiteSpace($Token)) {
        $requestHeaders["Authorization"] = "Bearer $Token"
    }

    $params = @{
        Method = $Method
        Uri = $uri
        Headers = $requestHeaders
        UseBasicParsing = $true
        TimeoutSec = 30
    }

    if ($null -ne $Body) {
        $params["ContentType"] = "application/json; charset=utf-8"
        $params["Body"] = ($Body | ConvertTo-Json -Depth 12)
    }

    try {
        $resp = Invoke-WebRequest @params
        $text = [string]$resp.Content
        return [pscustomobject]@{
            StatusCode = [int]$resp.StatusCode
            Text = $text
            Json = Convert-JsonSafe $text
        }
    } catch {
        $response = $_.Exception.Response
        $statusCode = 0
        if ($response -and $response.StatusCode) {
            $statusCode = [int]$response.StatusCode
        }
        $text = Read-ErrorBody $response
        return [pscustomobject]@{
            StatusCode = $statusCode
            Text = $text
            Json = Convert-JsonSafe $text
        }
    }
}

function Invoke-MultipartUpload {
    param(
        [string]$Path,
        [string]$Token,
        [string]$FilePath,
        [string]$ContentType = "application/pdf"
    )

    Add-Type -AssemblyName System.Net.Http
    $client = New-Object System.Net.Http.HttpClient
    try {
        $client.DefaultRequestHeaders.Authorization =
            New-Object System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", $Token)

        $bytes = [System.IO.File]::ReadAllBytes($FilePath)
        $fileContent = New-Object System.Net.Http.ByteArrayContent(,$bytes)
        $fileContent.Headers.ContentType =
            New-Object System.Net.Http.Headers.MediaTypeHeaderValue($ContentType)

        $multipart = New-Object System.Net.Http.MultipartFormDataContent
        $multipart.Add($fileContent, "file", [System.IO.Path]::GetFileName($FilePath))

        $resp = $client.PostAsync("$BaseUrl$Path", $multipart).GetAwaiter().GetResult()
        $text = $resp.Content.ReadAsStringAsync().GetAwaiter().GetResult()

        return [pscustomobject]@{
            StatusCode = [int]$resp.StatusCode
            Text = $text
            Json = Convert-JsonSafe $text
        }
    } finally {
        if ($multipart) { $multipart.Dispose() }
        if ($fileContent) { $fileContent.Dispose() }
        $client.Dispose()
    }
}

function Expect-Status {
    param(
        [string]$Id,
        [string]$Name,
        [object]$Response,
        [int[]]$Expected,
        [string]$ExpectedCode = ""
    )

    $ok = $Expected -contains $Response.StatusCode
    if ($ok -and -not [string]::IsNullOrWhiteSpace($ExpectedCode)) {
        $ok = $Response.Json -and $Response.Json.code -eq $ExpectedCode
    }

    if ($ok) {
        $detail = "HTTP $($Response.StatusCode)"
        if ($Response.Json -and $Response.Json.code) {
            $detail = "$detail code=$($Response.Json.code)"
        }
        Add-Result $Id $Name "PASS" $detail
    } else {
        $detail = "HTTP $($Response.StatusCode)"
        if ($Response.Json -and $Response.Json.code) {
            $detail = "$detail code=$($Response.Json.code)"
        }
        if (-not [string]::IsNullOrWhiteSpace($Response.Text)) {
            $brief = $Response.Text
            if ($brief.Length -gt 300) { $brief = $brief.Substring(0, 300) }
            $detail = "$detail body=$brief"
        }
        Add-Result $Id $Name "FAIL" $detail
    }
}

function Require-JsonField {
    param(
        [string]$Id,
        [string]$Name,
        [object]$Value
    )
    if ($null -eq $Value -or ([string]$Value).Length -eq 0) {
        Add-Result $Id $Name "FAIL" "missing field"
        throw "$Id failed: missing required response field"
    }
}

function New-PostBody {
    param([string]$Title, [string]$Description)

    return @{
        sceneType = "INNOVATION_PROJECT"
        title = $Title
        description = $Description
        timeMode = "TEXT_PREFERENCE"
        timeText = "weekday evening"
        locationText = "online"
        participantCount = 3
        targetRequirement = "API smoke tester"
        contactPreference = "in-app chat first"
        tags = @("smoke", "api")
        attachmentIds = @()
        scenePayload = @{ projectDirection = "campus buddy smoke" }
    }
}

function Find-ConversationWith {
    param(
        [string]$Token,
        [string]$OtherUserId
    )

    $resp = Invoke-Api -Method Get -Path "/me/conversations?page=0&size=50" -Token $Token
    if ($resp.StatusCode -ne 200 -or -not $resp.Json -or -not $resp.Json.items) {
        return $null
    }
    foreach ($item in $resp.Json.items) {
        if ($item.otherParticipantId -eq $OtherUserId) {
            return $item
        }
    }
    return $null
}

Write-Host "base_url=$BaseUrl"
Write-Host "credentials=environment-variables"

$health = Invoke-Api -Method Get -Path "/health"
Expect-Status "SYS-001" "GET /health" $health @(200)

$info = Invoke-Api -Method Get -Path "/system/info"
Expect-Status "SYS-002" "GET /system/info" $info @(200)

$secureNoToken = Invoke-Api -Method Get -Path "/probe/secure"
Expect-Status "SEC-001" "GET /probe/secure without token" $secureNoToken @(401) "UNAUTHORIZED"

$badLogin = Invoke-Api -Method Post -Path "/auth/login" -Body @{
    campusEmail = $env:CAMPUS_BUDDY_SMOKE_EMAIL
    password = "definitely-wrong-password"
    clientName = "powershell-smoke"
    deviceId = "negative"
}
Expect-Status "SEC-006" "POST /auth/login wrong password" $badLogin @(401) "INVALID_LOGIN_CREDENTIALS"

$studentLogin = Invoke-Api -Method Post -Path "/auth/login" -Body @{
    campusEmail = $env:CAMPUS_BUDDY_SMOKE_EMAIL
    password = $env:CAMPUS_BUDDY_SMOKE_PASSWORD
    clientName = "powershell-smoke"
    deviceId = "student-$Suffix"
}
Expect-Status "SEC-002" "POST /auth/login student" $studentLogin @(200)
Require-JsonField "SEC-002A" "student accessToken" $studentLogin.Json.accessToken
$studentToken = $studentLogin.Json.accessToken
$studentUserId = $studentLogin.Json.user.userId

$adminLogin = Invoke-Api -Method Post -Path "/auth/login" -Body @{
    campusEmail = $env:CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL
    password = $env:CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD
    clientName = "powershell-smoke"
    deviceId = "admin-$Suffix"
}
Expect-Status "SEC-003" "POST /auth/login admin" $adminLogin @(200)
Require-JsonField "SEC-003A" "admin accessToken" $adminLogin.Json.accessToken
$adminToken = $adminLogin.Json.accessToken
$adminUserId = $adminLogin.Json.user.userId
if ($adminLogin.Json.user.accountRole -eq "ADMIN") {
    Add-Result "SEC-003B" "admin accountRole" "PASS" "ADMIN"
} else {
    Add-Result "SEC-003B" "admin accountRole" "FAIL" "role=$($adminLogin.Json.user.accountRole)"
}

$secureStudent = Invoke-Api -Method Get -Path "/probe/secure" -Token $studentToken
Expect-Status "SEC-004" "GET /probe/secure with student token" $secureStudent @(200)

$studentAdminForbidden = Invoke-Api -Method Get -Path "/admin/partner-posts/review-queue?page=0&size=1" -Token $studentToken
Expect-Status "SEC-005" "student cannot access admin queue" $studentAdminForbidden @(403) "FORBIDDEN"

$badEmailCode = Invoke-Api -Method Post -Path "/auth/campus-email/verification-codes" -Body @{
    campusEmail = "not-a-campus@example.invalid"
    purpose = "REGISTER_OR_LOGIN"
}
Expect-Status "AUTH-001" "send code with invalid domain" $badEmailCode @(400)

$badVerify = Invoke-Api -Method Post -Path "/auth/campus-email/verifications" -Body @{
    campusEmail = $env:CAMPUS_BUDDY_SMOKE_EMAIL
    code = "000000"
    purpose = "REGISTER_OR_LOGIN"
}
Expect-Status "AUTH-002" "verify wrong email code" $badVerify @(400, 409)

$badRegister = Invoke-Api -Method Post -Path "/auth/register" -Body @{
    campusEmail = "invalid-register@example.invalid"
    verificationTicket = "invalid-ticket"
    password = "InvalidLocalOnlyPass2026!"
    displayName = "Invalid Register"
}
Expect-Status "AUTH-003" "register with invalid domain/ticket" $badRegister @(400, 409)

$identityMe = Invoke-Api -Method Get -Path "/auth/identity-verifications/me" -Token $studentToken
Expect-Status "IDV-001" "GET /auth/identity-verifications/me" $identityMe @(200)

$tempPdf = Join-Path ([System.IO.Path]::GetTempPath()) "campus-buddy-smoke-$Suffix.pdf"
Set-Content -LiteralPath $tempPdf -Encoding ASCII -Value "%PDF-1.4`n1 0 obj<<>>endobj`ntrailer<<>>`n%%EOF"
$upload = Invoke-MultipartUpload -Path "/auth/identity-verifications/materials" -Token $studentToken -FilePath $tempPdf
Expect-Status "IDV-002" "POST /auth/identity-verifications/materials" $upload @(200)
$attachmentId = if ($upload.Json) { $upload.Json.attachmentId } else { $null }
if ($attachmentId) {
    $deleteAttachment = Invoke-Api -Method Delete -Path "/auth/identity-verifications/materials/$attachmentId" -Token $studentToken
    Expect-Status "IDV-003" "DELETE uploaded identity material" $deleteAttachment @(204)
} else {
    Add-Result "IDV-003" "DELETE uploaded identity material" "SKIP" "upload did not return attachmentId"
}
Remove-Item -LiteralPath $tempPdf -Force -ErrorAction SilentlyContinue

$identityResubmit = Invoke-Api -Method Post -Path "/auth/identity-verifications" -Token $studentToken -Body @{
    realName = "Smoke Student"
    studentNumber = "SMOKE$Suffix"
    college = "Smoke College"
    major = "Smoke Major"
    grade = "2026"
}
Expect-Status "IDV-004" "POST identity verification with existing account state" $identityResubmit @(200, 409)

$identityQueue = Invoke-Api -Method Get -Path "/admin/identity-verifications?status=PENDING_REVIEW" -Token $adminToken
Expect-Status "IDV-005" "GET admin identity verification queue" $identityQueue @(200)
if ($identityQueue.StatusCode -eq 200 -and $identityQueue.Json -and $identityQueue.Json.Count -gt 0) {
    $candidate = $identityQueue.Json | Where-Object { $_.materialAttachmentId } | Select-Object -First 1
    if ($candidate) {
        $material = Invoke-Api -Method Get -Path "/admin/identity-verifications/$($candidate.submissionId)/material" -Token $adminToken
        Expect-Status "IDV-006" "GET admin identity material" $material @(200)
    } else {
        Add-Result "IDV-006" "GET admin identity material" "SKIP" "queue has no item with materialAttachmentId"
    }
} else {
    Add-Result "IDV-006" "GET admin identity material" "SKIP" "identity review queue is empty"
}

$reviewSubmissionId = $env:CAMPUS_BUDDY_IDENTITY_REVIEW_SUBMISSION_ID
if ($AllowIdentityReviewMutation -and -not [string]::IsNullOrWhiteSpace($reviewSubmissionId)) {
    $identityReview = Invoke-Api -Method Post -Path "/admin/identity-verifications/$reviewSubmissionId/reviews" -Token $adminToken -Body @{
        decision = "REJECTED"
        rejectReason = "smoke controlled rejection"
    }
    Expect-Status "IDV-007" "POST admin identity review controlled submission" $identityReview @(200)
} else {
    Add-Result "IDV-007" "POST admin identity review" "SKIP" "requires -AllowIdentityReviewMutation and CAMPUS_BUDDY_IDENTITY_REVIEW_SUBMISSION_ID"
}

$incompleteDraft = Invoke-Api -Method Post -Path "/me/partner-posts" -Token $studentToken -Body @{
    sceneType = "INNOVATION_PROJECT"
}
Expect-Status "POST-001" "POST incomplete draft" $incompleteDraft @(200)
if ($incompleteDraft.StatusCode -eq 200 -and $incompleteDraft.Json.postId) {
    $badSubmit = Invoke-Api -Method Post -Path "/me/partner-posts/$($incompleteDraft.Json.postId)/submit-review" -Token $studentToken
    Expect-Status "POST-002" "submit incomplete draft should fail" $badSubmit @(400) "VALIDATION_FAILED"
} else {
    Add-Result "POST-002" "submit incomplete draft should fail" "SKIP" "incomplete draft not created"
}

$rejectDraft = Invoke-Api -Method Post -Path "/me/partner-posts" -Token $studentToken -Body (New-PostBody "smoke reject $Suffix" "smoke reject flow")
Expect-Status "POST-003R" "POST complete draft for reject" $rejectDraft @(200)
$rejectPostId = $rejectDraft.Json.postId
if ($rejectPostId) {
    $submitReject = Invoke-Api -Method Post -Path "/me/partner-posts/$rejectPostId/submit-review" -Token $studentToken
    Expect-Status "POST-007R" "submit reject-flow draft" $submitReject @(200)
    $adminRejectDetail = Invoke-Api -Method Get -Path "/admin/partner-posts/$rejectPostId" -Token $adminToken
    Expect-Status "ADMPOST-002R" "GET admin post detail for reject" $adminRejectDetail @(200)
    $adminReject = Invoke-Api -Method Post -Path "/admin/partner-posts/$rejectPostId/review" -Token $adminToken -Body @{
        decision = "REJECT"
        reason = "smoke reject"
    }
    Expect-Status "ADMPOST-003" "POST admin reject partner post" $adminReject @(200)
}

$draft = Invoke-Api -Method Post -Path "/me/partner-posts" -Token $studentToken -Body (New-PostBody "smoke approve $Suffix" "smoke approve flow")
Expect-Status "POST-003" "POST complete draft for approve" $draft @(200)
Require-JsonField "POST-003A" "approved-flow postId" $draft.Json.postId
$postId = $draft.Json.postId

$updated = Invoke-Api -Method Put -Path "/me/partner-posts/$postId" -Token $studentToken -Body (New-PostBody "smoke ok $Suffix" "smoke updated approve flow")
Expect-Status "POST-004" "PUT my draft" $updated @(200)

$myPosts = Invoke-Api -Method Get -Path "/me/partner-posts?page=0&size=20" -Token $studentToken
Expect-Status "POST-005" "GET my partner posts" $myPosts @(200)

$myPostDetail = Invoke-Api -Method Get -Path "/me/partner-posts/$postId" -Token $studentToken
Expect-Status "POST-006" "GET my post detail" $myPostDetail @(200)

$submit = Invoke-Api -Method Post -Path "/me/partner-posts/$postId/submit-review" -Token $studentToken
Expect-Status "POST-007" "POST submit review" $submit @(200)

$queue = Invoke-Api -Method Get -Path "/admin/partner-posts/review-queue?page=0&size=20" -Token $adminToken
Expect-Status "ADMPOST-001" "GET admin partner-post review queue" $queue @(200)

$adminDetail = Invoke-Api -Method Get -Path "/admin/partner-posts/$postId" -Token $adminToken
Expect-Status "ADMPOST-002" "GET admin partner-post detail" $adminDetail @(200)

$approve = Invoke-Api -Method Post -Path "/admin/partner-posts/$postId/review" -Token $adminToken -Body @{
    decision = "APPROVE"
    reason = $null
}
Expect-Status "ADMPOST-004" "POST admin approve partner post" $approve @(200)

$plaza = Invoke-Api -Method Get -Path "/partner-posts?page=0&size=20" -Token $studentToken
Expect-Status "PLAZA-001" "GET plaza list" $plaza @(200)

$plazaDetail = Invoke-Api -Method Get -Path "/partner-posts/$postId" -Token $studentToken
Expect-Status "PLAZA-002" "GET plaza post detail" $plazaDetail @(200)

$contact = Invoke-Api -Method Post -Path "/partner-posts/$postId/contact-requests" -Token $adminToken -Body @{
    message = "hello $Suffix"
}
if ($contact.StatusCode -eq 200) {
    Add-Result "CHAT-001" "POST contact request" "PASS" "conversationId=$($contact.Json.conversationId)"
    $conversationId = $contact.Json.conversationId
} elseif ($contact.StatusCode -eq 403 -and $contact.Json -and $contact.Json.code -eq "CONTACT_REPLY_REQUIRED") {
    Add-Result "CHAT-001" "POST contact request" "PASS" "CONTACT_REPLY_REQUIRED on existing conversation"
    $existing = Find-ConversationWith -Token $adminToken -OtherUserId $studentUserId
    if ($existing) {
        $conversationId = $existing.conversationId
    }
} else {
    Expect-Status "CHAT-001" "POST contact request" $contact @(200)
}

if (-not $conversationId) {
    Add-Result "CHAT-SETUP" "conversation setup" "FAIL" "no conversationId available"
} else {
    $studentConversations = Invoke-Api -Method Get -Path "/me/conversations?page=0&size=50" -Token $studentToken
    Expect-Status "CHAT-002S" "GET student conversations" $studentConversations @(200)

    $adminConversations = Invoke-Api -Method Get -Path "/me/conversations?page=0&size=50" -Token $adminToken
    Expect-Status "CHAT-002A" "GET admin conversations" $adminConversations @(200)

    $messages = Invoke-Api -Method Get -Path "/me/conversations/$conversationId/messages?page=0&size=20" -Token $studentToken
    Expect-Status "CHAT-003" "GET conversation messages" $messages @(200)

    $studentMsg1 = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/messages" -Token $studentToken -Body @{ message = "s1 $Suffix" }
    Expect-Status "CHAT-004S1" "student send message" $studentMsg1 @(200, 403)

    $adminMsg1 = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/messages" -Token $adminToken -Body @{ message = "a1 $Suffix" }
    Expect-Status "CHAT-004A1" "admin send message" $adminMsg1 @(200, 403)

    $studentMsg2 = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/messages" -Token $studentToken -Body @{ message = "s2 $Suffix" }
    Expect-Status "CHAT-004S2" "student send second message" $studentMsg2 @(200, 403)

    $adminMsg2 = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/messages" -Token $adminToken -Body @{ message = "a2 $Suffix" }
    Expect-Status "CHAT-004A2" "admin send second message" $adminMsg2 @(200, 403)

    $afterId = 0
    if ($studentMsg1.StatusCode -eq 200 -and $studentMsg1.Json.messageId) {
        $afterId = [int64]$studentMsg1.Json.messageId
    }
    $incremental = Invoke-Api -Method Get -Path "/me/conversations/$conversationId/messages?afterMessageId=$afterId&size=10" -Token $studentToken
    Expect-Status "CHAT-005" "GET incremental messages" $incremental @(200)

    $read = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/read" -Token $studentToken
    Expect-Status "CHAT-006" "POST mark conversation read" $read @(200)

    $studentCardGet = Invoke-Api -Method Get -Path "/me/contact-card" -Token $studentToken
    Expect-Status "CARD-001S" "GET student contact card" $studentCardGet @(200)

    $studentCardPut = Invoke-Api -Method Put -Path "/me/contact-card" -Token $studentToken -Body @{
        wechatId = "smoke_student_$Suffix"
        phoneNumber = "13800001111"
        qqNumber = "10000$Suffix"
    }
    Expect-Status "CARD-002S" "PUT student contact card" $studentCardPut @(200)

    $adminCardPut = Invoke-Api -Method Put -Path "/me/contact-card" -Token $adminToken -Body @{
        wechatId = "smoke_admin_$Suffix"
        phoneNumber = "13800002222"
        qqNumber = "20000$Suffix"
    }
    Expect-Status "CARD-002A" "PUT admin contact card" $adminCardPut @(200)

    $unlockStatus = Invoke-Api -Method Get -Path "/me/conversations/$conversationId/contact-unlock" -Token $studentToken
    Expect-Status "UNLOCK-001" "GET contact unlock status" $unlockStatus @(200)

    $studentConfirm = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/contact-unlock/confirm" -Token $studentToken
    Expect-Status "UNLOCK-002S" "student confirm contact unlock" $studentConfirm @(200)

    $adminConfirm = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/contact-unlock/confirm" -Token $adminToken
    Expect-Status "UNLOCK-002A" "admin confirm contact unlock" $adminConfirm @(200)

    $peerCard = Invoke-Api -Method Get -Path "/me/conversations/$conversationId/peer-contact-card" -Token $studentToken
    Expect-Status "UNLOCK-003" "GET peer contact card" $peerCard @(200)

    $studentConvItem = Find-ConversationWith -Token $studentToken -OtherUserId $adminUserId
    $revieweeId = if ($studentConvItem -and $studentConvItem.otherParticipantId) { $studentConvItem.otherParticipantId } else { $adminUserId }
    $review = Invoke-Api -Method Post -Path "/me/reviews" -Token $studentToken -Body @{
        conversationId = [int64]$conversationId
        revieweeId = $revieweeId
        rating = 5
        reviewTags = @()
    }
    if ($review.StatusCode -eq 201) {
        Add-Result "REV-001" "POST create review" "PASS" "reviewId=$($review.Json.id)"
        $reviewId = $review.Json.id
    } elseif ($review.StatusCode -eq 409 -and $review.Json -and $review.Json.code -eq "REVIEW_ALREADY_EXISTS") {
        Add-Result "REV-001" "POST create review" "SKIP" "review already exists for this conversation"
    } else {
        Expect-Status "REV-001" "POST create review" $review @(201)
    }

    if ($reviewId) {
        $reviewUpdate = Invoke-Api -Method Put -Path "/me/reviews/$reviewId" -Token $studentToken -Body @{
            rating = 4
            reviewTags = @()
        }
        Expect-Status "REV-002" "PUT update review" $reviewUpdate @(200)
    } else {
        Add-Result "REV-002" "PUT update review" "SKIP" "no newly created reviewId"
    }

    $given = Invoke-Api -Method Get -Path "/me/reviews/given?page=0&size=20" -Token $studentToken
    Expect-Status "REV-003" "GET given reviews" $given @(200)

    $received = Invoke-Api -Method Get -Path "/me/reviews/received?page=0&size=20" -Token $adminToken
    Expect-Status "REV-004" "GET received reviews" $received @(200)

    $myCredit = Invoke-Api -Method Get -Path "/me/credit-summary" -Token $studentToken
    Expect-Status "CREDIT-001" "GET my credit summary" $myCredit @(200)

    $publicCredit = Invoke-Api -Method Get -Path "/users/$adminUserId/credit-summary" -Token $studentToken
    Expect-Status "CREDIT-002" "GET public credit summary" $publicCredit @(200)

    $close = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/close" -Token $studentToken
    Expect-Status "CHAT-007" "POST close conversation" $close @(200)

    $sendClosed = Invoke-Api -Method Post -Path "/me/conversations/$conversationId/messages" -Token $studentToken -Body @{ message = "closed $Suffix" }
    Expect-Status "CHAT-008" "send message after close should fail" $sendClosed @(403) "CONVERSATION_CLOSED"

    $recontact = Invoke-Api -Method Post -Path "/partner-posts/$postId/contact-requests" -Token $adminToken -Body @{
        message = "again $Suffix"
    }
    Expect-Status "CHAT-009" "recontact after close" $recontact @(200, 403)
}

$unpublish = Invoke-Api -Method Post -Path "/me/partner-posts/$postId/unpublish" -Token $studentToken
Expect-Status "POST-009" "POST unpublish approved post" $unpublish @(200)

Write-Host ""
Write-Host "===== SUMMARY ====="
$script:Results | Group-Object Status | Sort-Object Name | ForEach-Object {
    Write-Host ("{0}: {1}" -f $_.Name, $_.Count)
}

$failed = @($script:Results | Where-Object { $_.Status -eq "FAIL" })
if ($failed.Count -gt 0) {
    Write-Host ""
    Write-Host "Failed cases:"
    $failed | Format-Table Id, Name, Detail -AutoSize
    exit 1
}

exit 0
