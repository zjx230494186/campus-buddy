#pragma once

#include <functional>

#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/SecureTokenStore.h"
#include "domain/ReviewCreditModels.h"

class ReviewCreditApiService : public QObject
{
public:
    using CreateReviewCallback = std::function<void(const ReviewResult &)>;
    using UpdateReviewCallback = std::function<void(const ReviewResult &)>;
    using ReviewListCallback = std::function<void(const ReviewListResult &)>;
    using MyCreditSummaryCallback = std::function<void(const MyCreditSummaryResult &)>;
    using PublicCreditSummaryCallback = std::function<void(const PublicCreditSummaryResult &)>;

    explicit ReviewCreditApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent = nullptr);

    void createReview(const CreateReviewRequest &request, CreateReviewCallback callback);
    void updateReview(long long reviewId, const UpdateReviewRequest &request, UpdateReviewCallback callback);
    void listGivenReviews(int page, int size, ReviewListCallback callback);
    void listReceivedReviews(int page, int size, ReviewListCallback callback);
    void getMyCreditSummary(MyCreditSummaryCallback callback);
    void getPublicCreditSummary(const QString &userId, PublicCreditSummaryCallback callback);

private:
    static ReviewItem parseReviewItem(const QJsonObject &obj);

    CampusApiClient &client_;
    SecureTokenStore &tokenStore_;
};
