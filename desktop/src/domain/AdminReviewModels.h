#pragma once

#include <QString>
#include <QStringList>

struct PartnerPostReviewQueueItem
{
    QString postId;
    QString publisherId;
    QString publisherDisplayName;
    QString sceneType;
    QString status;
    QString title;
    QString summary;
    QString timeText;
    QString locationText;
    QString updatedAt;
};

struct PartnerPostReviewQueueResult
{
    bool success = false;
    QList<PartnerPostReviewQueueItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
};

struct PartnerPostAdminDetail
{
    QString postId;
    QString publisherId;
    QString publisherDisplayName;
    QString publisherAuthenticationStatus;
    QString sceneType;
    QString status;
    QString title;
    QString description;
    QString timeMode;
    QString timeText;
    QString startAt;
    QString endAt;
    QString locationText;
    int participantCount = 0;
    QString targetRequirement;
    QStringList tags;
    QString scenePayload;
    QString rejectReason;
    QString reviewedBy;
    QString reviewedAt;
    QString publishedAt;
    QString createdAt;
    QString updatedAt;
};

struct AdminPostDetailResult
{
    bool success = false;
    PartnerPostAdminDetail detail;
    QString errorCode;
    QString errorMessage;
};

struct PartnerPostReviewRequest
{
    QString decision;
    QString reason;
};

struct PartnerPostReviewResult
{
    bool success = false;
    PartnerPostAdminDetail detail;
    QString errorCode;
    QString errorMessage;
};

struct PendingIdentityVerificationItem
{
    long long submissionId = 0;
    QString userId;
    QString realName;
    QString studentNumber;
    QString college;
    QString major;
    QString grade;
    QString reviewStatus;
    QString submittedAt;
    QString materialAttachmentId;
    QString materialContentType;
    long long materialSizeBytes = 0;
};

struct PendingIdentityVerificationListResult
{
    bool success = false;
    QList<PendingIdentityVerificationItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
};

struct IdentityVerificationReviewRequest
{
    QString decision;
    QString rejectReason;
};

struct IdentityVerificationReviewResult
{
    bool success = false;
    QString reviewStatus;
    QString authenticationStatus;
    QString reviewedAt;
    QString rejectReason;
    QString errorCode;
    QString errorMessage;
};
