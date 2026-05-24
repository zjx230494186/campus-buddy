#pragma once

#include <functional>

#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/SecureTokenStore.h"
#include "domain/AdminReviewModels.h"

class AdminReviewApiService : public QObject
{
public:
    using ReviewQueueCallback = std::function<void(const PartnerPostReviewQueueResult &)>;
    using AdminPostDetailCallback = std::function<void(const AdminPostDetailResult &)>;
    using PartnerPostReviewCallback = std::function<void(const PartnerPostReviewResult &)>;
    using PendingIdentityListCallback = std::function<void(const PendingIdentityVerificationListResult &)>;
    using IdentityReviewCallback = std::function<void(const IdentityVerificationReviewResult &)>;

    explicit AdminReviewApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent = nullptr);

    void listPartnerPostReviewQueue(int page, int size, ReviewQueueCallback callback);
    void getPartnerPostAdminDetail(const QString &postId, AdminPostDetailCallback callback);
    void reviewPartnerPost(const QString &postId, const PartnerPostReviewRequest &request, PartnerPostReviewCallback callback);
    void listPendingIdentityVerifications(int page, int size, PendingIdentityListCallback callback);
    void reviewIdentityVerification(const QString &submissionId, const IdentityVerificationReviewRequest &request, IdentityReviewCallback callback);

private:
    static PartnerPostReviewQueueItem parseQueueItem(const QJsonObject &obj);
    static PartnerPostAdminDetail parseAdminDetail(const QJsonObject &obj);
    static PendingIdentityVerificationItem parsePendingItem(const QJsonObject &obj);

    CampusApiClient &client_;
    SecureTokenStore &tokenStore_;
};
