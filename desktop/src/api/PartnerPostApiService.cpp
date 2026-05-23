#include "api/PartnerPostApiService.h"

#include <QJsonArray>

PartnerPostApiService::PartnerPostApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent)
    : QObject(parent),
      client_(client),
      tokenStore_(tokenStore)
{
}

void PartnerPostApiService::listPosts(int page, int size, PlazaListCallback callback)
{
    listPosts(QString(), QString(), page, size, std::move(callback));
}

void PartnerPostApiService::listPosts(const QString &sceneType, const QString &keyword, int page, int size, PlazaListCallback callback)
{
    QString path = QStringLiteral("/partner-posts?page=%1&size=%2").arg(page).arg(size);
    if (!sceneType.isEmpty()) {
        path += QStringLiteral("&sceneType=%1").arg(sceneType);
    }
    if (!keyword.isEmpty()) {
        path += QStringLiteral("&keyword=%1").arg(keyword);
    }

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        PlazaListResult result;
        if (response.ok) {
            result.success = true;
            result.page = response.json.value(QStringLiteral("page")).toInt();
            result.size = response.json.value(QStringLiteral("size")).toInt();
            result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
            result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();

            const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
            for (const QJsonValue &v : items) {
                result.items.append(parseListItem(v.toObject()));
            }
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void PartnerPostApiService::getPostDetail(const QString &postId, PlazaDetailCallback callback)
{
    QString path = QStringLiteral("/partner-posts/%1").arg(postId);

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        PlazaDetailResult result;
        if (response.ok) {
            result = parseDetailResponse(response.json);
            result.success = true;
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

PublicCreditSummary PartnerPostApiService::parseCreditSummary(const QJsonObject &obj)
{
    PublicCreditSummary summary;
    summary.averageRating = obj.value(QStringLiteral("averageRating")).toDouble();
    summary.ratingSampleCount = obj.value(QStringLiteral("ratingSampleCount")).toInt();
    summary.realConversationCount = obj.value(QStringLiteral("realConversationCount")).toInt();
    summary.updatedAt = obj.value(QStringLiteral("updatedAt")).toString();

    const QJsonArray tags = obj.value(QStringLiteral("topTags")).toArray();
    for (const QJsonValue &t : tags) {
        summary.topTags.append(t.toString());
    }
    return summary;
}

PlazaListItem PartnerPostApiService::parseListItem(const QJsonObject &obj)
{
    PlazaListItem item;
    item.postId = obj.value(QStringLiteral("postId")).toString();
    item.publisherId = obj.value(QStringLiteral("publisherId")).toString();
    item.publisherDisplayName = obj.value(QStringLiteral("publisherDisplayName")).toString();
    item.publisherAuthenticationStatus = obj.value(QStringLiteral("publisherAuthenticationStatus")).toString();
    item.sceneType = obj.value(QStringLiteral("sceneType")).toString();
    item.status = obj.value(QStringLiteral("status")).toString();
    item.title = obj.value(QStringLiteral("title")).toString();
    item.description = obj.value(QStringLiteral("description")).toString();
    item.timeText = obj.value(QStringLiteral("timeText")).toString();
    item.locationText = obj.value(QStringLiteral("locationText")).toString();
    item.publishedAt = obj.value(QStringLiteral("publishedAt")).toString();
    item.updatedAt = obj.value(QStringLiteral("updatedAt")).toString();
    item.ownPost = obj.value(QStringLiteral("ownPost")).toBool();

    const QJsonArray tags = obj.value(QStringLiteral("tags")).toArray();
    for (const QJsonValue &t : tags) {
        item.tags.append(t.toString());
    }

    if (obj.contains(QStringLiteral("publisherCreditSummary")) && obj.value(QStringLiteral("publisherCreditSummary")).isObject()) {
        item.publisherCreditSummary = parseCreditSummary(obj.value(QStringLiteral("publisherCreditSummary")).toObject());
    }

    return item;
}

PlazaDetailResult PartnerPostApiService::parseDetailResponse(const QJsonObject &json)
{
    PlazaDetailResult result;
    result.postId = json.value(QStringLiteral("postId")).toString();
    result.publisherId = json.value(QStringLiteral("publisherId")).toString();
    result.publisherDisplayName = json.value(QStringLiteral("publisherDisplayName")).toString();
    result.publisherAuthenticationStatus = json.value(QStringLiteral("publisherAuthenticationStatus")).toString();
    result.sceneType = json.value(QStringLiteral("sceneType")).toString();
    result.status = json.value(QStringLiteral("status")).toString();
    result.title = json.value(QStringLiteral("title")).toString();
    result.description = json.value(QStringLiteral("description")).toString();
    result.timeMode = json.value(QStringLiteral("timeMode")).toString();
    result.timeText = json.value(QStringLiteral("timeText")).toString();
    result.startAt = json.value(QStringLiteral("startAt")).toString();
    result.endAt = json.value(QStringLiteral("endAt")).toString();
    result.locationText = json.value(QStringLiteral("locationText")).toString();
    result.participantCount = json.value(QStringLiteral("participantCount")).toInt();
    result.targetRequirement = json.value(QStringLiteral("targetRequirement")).toString();
    result.publishedAt = json.value(QStringLiteral("publishedAt")).toString();
    result.updatedAt = json.value(QStringLiteral("updatedAt")).toString();
    result.ownPost = json.value(QStringLiteral("ownPost")).toBool();

    const QJsonArray tags = json.value(QStringLiteral("tags")).toArray();
    for (const QJsonValue &t : tags) {
        result.tags.append(t.toString());
    }

    if (json.contains(QStringLiteral("publisherCreditSummary")) && json.value(QStringLiteral("publisherCreditSummary")).isObject()) {
        result.publisherCreditSummary = parseCreditSummary(json.value(QStringLiteral("publisherCreditSummary")).toObject());
    }

    return result;
}
