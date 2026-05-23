#include "api/ReviewCreditApiService.h"

#include <QJsonArray>
#include <QUrlQuery>

ReviewCreditApiService::ReviewCreditApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent)
    : QObject(parent),
      client_(client),
      tokenStore_(tokenStore)
{
}

void ReviewCreditApiService::createReview(const CreateReviewRequest &request, CreateReviewCallback callback)
{
    QJsonObject body;
    body[QStringLiteral("conversationId")] = request.conversationId;
    body[QStringLiteral("revieweeId")] = request.revieweeId;
    body[QStringLiteral("rating")] = request.rating;

    QJsonArray tagsArray;
    for (const QString &tag : request.reviewTags) {
        tagsArray.append(tag);
    }
    body[QStringLiteral("reviewTags")] = tagsArray;

    client_.postJson(QStringLiteral("/me/reviews"), body, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            ReviewResult result;
            if (response.ok) {
                result.success = true;
                result.review = parseReviewItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void ReviewCreditApiService::updateReview(long long reviewId, const UpdateReviewRequest &request, UpdateReviewCallback callback)
{
    QString path = QStringLiteral("/me/reviews/%1").arg(reviewId);

    QJsonObject body;
    body[QStringLiteral("rating")] = request.rating;

    QJsonArray tagsArray;
    for (const QString &tag : request.reviewTags) {
        tagsArray.append(tag);
    }
    body[QStringLiteral("reviewTags")] = tagsArray;

    client_.putJson(path, body, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            ReviewResult result;
            if (response.ok) {
                result.success = true;
                result.review = parseReviewItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void ReviewCreditApiService::listGivenReviews(int page, int size, ReviewListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/me/reviews/given?") + query.toString();

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            ReviewListResult result;
            if (response.ok) {
                result.success = true;
                result.page = response.json.value(QStringLiteral("page")).toInt();
                result.size = response.json.value(QStringLiteral("size")).toInt();
                result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
                result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();
                const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
                for (const QJsonValue &v : items) {
                    result.items.append(parseReviewItem(v.toObject()));
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void ReviewCreditApiService::listReceivedReviews(int page, int size, ReviewListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/me/reviews/received?") + query.toString();

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            ReviewListResult result;
            if (response.ok) {
                result.success = true;
                result.page = response.json.value(QStringLiteral("page")).toInt();
                result.size = response.json.value(QStringLiteral("size")).toInt();
                result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
                result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();
                const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
                for (const QJsonValue &v : items) {
                    result.items.append(parseReviewItem(v.toObject()));
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void ReviewCreditApiService::getMyCreditSummary(MyCreditSummaryCallback callback)
{
    client_.getJson(QStringLiteral("/me/credit-summary"), tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            MyCreditSummaryResult result;
            if (response.ok) {
                result.success = true;
                result.userId = response.json.value(QStringLiteral("userId")).toString();
                result.averageRating = response.json.value(QStringLiteral("averageRating")).toDouble();
                result.realConversationCount = response.json.value(QStringLiteral("realConversationCount")).toInteger();
                result.ratingSampleCount = response.json.value(QStringLiteral("ratingSampleCount")).toInteger();
                result.disputedReviewCount = response.json.value(QStringLiteral("disputedReviewCount")).toInt();
                result.updatedAt = response.json.value(QStringLiteral("updatedAt")).toString();

                const QJsonArray tags = response.json.value(QStringLiteral("topTags")).toArray();
                for (const QJsonValue &t : tags) {
                    const QJsonObject tagObj = t.toObject();
                    CreditSummaryTagItem tagItem;
                    tagItem.tag = tagObj.value(QStringLiteral("tag")).toString();
                    tagItem.count = tagObj.value(QStringLiteral("count")).toInt();
                    result.topTags.append(tagItem);
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

void ReviewCreditApiService::getPublicCreditSummary(const QString &userId, PublicCreditSummaryCallback callback)
{
    QString path = QStringLiteral("/users/%1/credit-summary").arg(userId);

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PublicCreditSummaryResult result;
            if (response.ok) {
                result.success = true;
                result.userId = response.json.value(QStringLiteral("userId")).toString();
                result.averageRating = response.json.value(QStringLiteral("averageRating")).toDouble();
                result.realConversationCount = response.json.value(QStringLiteral("realConversationCount")).toInteger();
                result.ratingSampleCount = response.json.value(QStringLiteral("ratingSampleCount")).toInteger();
                result.updatedAt = response.json.value(QStringLiteral("updatedAt")).toString();

                const QJsonArray tags = response.json.value(QStringLiteral("topTags")).toArray();
                for (const QJsonValue &t : tags) {
                    const QJsonObject tagObj = t.toObject();
                    CreditSummaryTagItem tagItem;
                    tagItem.tag = tagObj.value(QStringLiteral("tag")).toString();
                    tagItem.count = tagObj.value(QStringLiteral("count")).toInt();
                    result.topTags.append(tagItem);
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
            }
            if (callback) callback(result);
        });
}

ReviewItem ReviewCreditApiService::parseReviewItem(const QJsonObject &obj)
{
    ReviewItem item;
    item.id = obj.value(QStringLiteral("id")).toInteger();
    item.conversationId = obj.value(QStringLiteral("conversationId")).toInteger();
    item.reviewerId = obj.value(QStringLiteral("reviewerId")).toString();
    item.revieweeId = obj.value(QStringLiteral("revieweeId")).toString();
    item.rating = obj.value(QStringLiteral("rating")).toInt();
    item.status = obj.value(QStringLiteral("status")).toString();
    item.modifiedOnce = obj.value(QStringLiteral("modifiedOnce")).toBool();
    item.createdAt = obj.value(QStringLiteral("createdAt")).toString();
    item.updatedAt = obj.value(QStringLiteral("updatedAt")).toString();

    const QJsonArray tags = obj.value(QStringLiteral("reviewTags")).toArray();
    for (const QJsonValue &t : tags) {
        item.reviewTags.append(t.toString());
    }
    return item;
}
