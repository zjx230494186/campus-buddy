#include "api/AdminReviewApiService.h"

#include <QJsonArray>
#include <QUrlQuery>

AdminReviewApiService::AdminReviewApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent)
    : QObject(parent),
      client_(client),
      tokenStore_(tokenStore)
{
}

void AdminReviewApiService::listPartnerPostReviewQueue(int page, int size, ReviewQueueCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/admin/partner-posts/review-queue?") + query.toString();

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PartnerPostReviewQueueResult result;
            if (response.ok) {
                result.success = true;
                result.page = response.json.value(QStringLiteral("page")).toInt();
                result.size = response.json.value(QStringLiteral("size")).toInt();
                result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
                result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();
                const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
                for (const QJsonValue &v : items) {
                    result.items.append(parseQueueItem(v.toObject()));
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void AdminReviewApiService::getPartnerPostAdminDetail(const QString &postId, AdminPostDetailCallback callback)
{
    QString path = QStringLiteral("/admin/partner-posts/%1").arg(postId);

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            AdminPostDetailResult result;
            if (response.ok) {
                result.success = true;
                result.detail = parseAdminDetail(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void AdminReviewApiService::reviewPartnerPost(const QString &postId, const PartnerPostReviewRequest &request, PartnerPostReviewCallback callback)
{
    QString path = QStringLiteral("/admin/partner-posts/%1/review").arg(postId);

    QJsonObject body;
    body[QStringLiteral("decision")] = request.decision;
    if (!request.reason.isEmpty()) {
        body[QStringLiteral("reason")] = request.reason;
    }

    client_.postJson(path, body, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PartnerPostReviewResult result;
            if (response.ok) {
                result.success = true;
                result.detail = parseAdminDetail(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void AdminReviewApiService::listPendingIdentityVerifications(int page, int size, PendingIdentityListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("status"), QStringLiteral("PENDING_REVIEW"));
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/admin/identity-verifications?") + query.toString();

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PendingIdentityVerificationListResult result;
            if (response.ok) {
                result.success = true;
                result.page = response.json.value(QStringLiteral("page")).toInt();
                result.size = response.json.value(QStringLiteral("size")).toInt();
                result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
                result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();
                const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
                for (const QJsonValue &v : items) {
                    result.items.append(parsePendingItem(v.toObject()));
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void AdminReviewApiService::reviewIdentityVerification(long long submissionId, const IdentityVerificationReviewRequest &request, IdentityReviewCallback callback)
{
    QString path = QStringLiteral("/admin/identity-verifications/%1/reviews").arg(submissionId);

    QJsonObject body;
    body[QStringLiteral("decision")] = request.decision;
    if (!request.rejectReason.isEmpty()) {
        body[QStringLiteral("rejectReason")] = request.rejectReason;
    }

    client_.postJson(path, body, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            IdentityVerificationReviewResult result;
            if (response.ok) {
                result.success = true;
                result.reviewStatus = response.json.value(QStringLiteral("reviewStatus")).toString();
                result.authenticationStatus = response.json.value(QStringLiteral("authenticationStatus")).toString();
                result.reviewedAt = response.json.value(QStringLiteral("reviewedAt")).toString();
                result.rejectReason = response.json.value(QStringLiteral("rejectReason")).toString();
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

PartnerPostReviewQueueItem AdminReviewApiService::parseQueueItem(const QJsonObject &obj)
{
    PartnerPostReviewQueueItem item;
    item.postId = obj.value(QStringLiteral("postId")).toString();
    item.publisherId = obj.value(QStringLiteral("publisherId")).toString();
    item.publisherDisplayName = obj.value(QStringLiteral("publisherDisplayName")).toString();
    item.sceneType = obj.value(QStringLiteral("sceneType")).toString();
    item.status = obj.value(QStringLiteral("status")).toString();
    item.title = obj.value(QStringLiteral("title")).toString();
    item.summary = obj.value(QStringLiteral("summary")).toString();
    item.timeText = obj.value(QStringLiteral("timeText")).toString();
    item.locationText = obj.value(QStringLiteral("locationText")).toString();
    item.updatedAt = obj.value(QStringLiteral("updatedAt")).toString();
    return item;
}

PartnerPostAdminDetail AdminReviewApiService::parseAdminDetail(const QJsonObject &obj)
{
    PartnerPostAdminDetail d;
    d.postId = obj.value(QStringLiteral("postId")).toString();
    d.publisherId = obj.value(QStringLiteral("publisherId")).toString();
    d.publisherDisplayName = obj.value(QStringLiteral("publisherDisplayName")).toString();
    d.publisherAuthenticationStatus = obj.value(QStringLiteral("publisherAuthenticationStatus")).toString();
    d.sceneType = obj.value(QStringLiteral("sceneType")).toString();
    d.status = obj.value(QStringLiteral("status")).toString();
    d.title = obj.value(QStringLiteral("title")).toString();
    d.description = obj.value(QStringLiteral("description")).toString();
    d.timeMode = obj.value(QStringLiteral("timeMode")).toString();
    d.timeText = obj.value(QStringLiteral("timeText")).toString();
    d.startAt = obj.value(QStringLiteral("startAt")).toString();
    d.endAt = obj.value(QStringLiteral("endAt")).toString();
    d.locationText = obj.value(QStringLiteral("locationText")).toString();
    d.participantCount = obj.value(QStringLiteral("participantCount")).toInt();
    d.targetRequirement = obj.value(QStringLiteral("targetRequirement")).toString();
    d.rejectReason = obj.value(QStringLiteral("rejectReason")).toString();
    d.reviewedBy = obj.value(QStringLiteral("reviewedBy")).toString();
    d.reviewedAt = obj.value(QStringLiteral("reviewedAt")).toString();
    d.publishedAt = obj.value(QStringLiteral("publishedAt")).toString();
    d.createdAt = obj.value(QStringLiteral("createdAt")).toString();
    d.updatedAt = obj.value(QStringLiteral("updatedAt")).toString();

    const QJsonArray tags = obj.value(QStringLiteral("tags")).toArray();
    for (const QJsonValue &t : tags) {
        d.tags.append(t.toString());
    }
    d.scenePayload = obj.value(QStringLiteral("scenePayload")).toString();
    return d;
}

PendingIdentityVerificationItem AdminReviewApiService::parsePendingItem(const QJsonObject &obj)
{
    PendingIdentityVerificationItem item;
    item.submissionId = obj.value(QStringLiteral("submissionId")).toInteger();
    item.userId = obj.value(QStringLiteral("userId")).toString();
    item.realName = obj.value(QStringLiteral("realName")).toString();
    item.studentNumber = obj.value(QStringLiteral("studentNumber")).toString();
    item.college = obj.value(QStringLiteral("college")).toString();
    item.major = obj.value(QStringLiteral("major")).toString();
    item.grade = obj.value(QStringLiteral("grade")).toString();
    item.reviewStatus = obj.value(QStringLiteral("reviewStatus")).toString();
    item.submittedAt = obj.value(QStringLiteral("submittedAt")).toString();
    item.materialAttachmentId = obj.value(QStringLiteral("materialAttachmentId")).toString();
    item.materialContentType = obj.value(QStringLiteral("materialContentType")).toString();
    item.materialSizeBytes = obj.value(QStringLiteral("materialSizeBytes")).toInteger();
    return item;
}
