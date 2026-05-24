#include "api/MyPartnerPostApiService.h"

#include <QJsonArray>
#include <QUrlQuery>

MyPartnerPostApiService::MyPartnerPostApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent)
    : QObject(parent),
      client_(client),
      tokenStore_(tokenStore)
{
}

void MyPartnerPostApiService::createDraft(const MyPostDraftRequest &request, CreateDraftCallback callback)
{
    QJsonObject body = buildDraftBody(request);

    client_.postJson(QStringLiteral("/me/partner-posts"), body, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            MyPostResult result;
            if (response.ok) {
                result.success = true;
                result.post = parsePostItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

void MyPartnerPostApiService::updateDraft(const QString &postId, const MyPostDraftRequest &request, UpdateDraftCallback callback)
{
    QString path = QStringLiteral("/me/partner-posts/%1").arg(postId);
    QJsonObject body = buildDraftBody(request);

    client_.putJson(path, body, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            MyPostResult result;
            if (response.ok) {
                result.success = true;
                result.post = parsePostItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

void MyPartnerPostApiService::listMyPosts(int page, int size, MyPostListCallback callback)
{
    listMyPosts(QString(), page, size, std::move(callback));
}

void MyPartnerPostApiService::listMyPosts(const QString &status, int page, int size, MyPostListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    if (!status.isEmpty()) {
        query.addQueryItem(QStringLiteral("status"), status);
    }

    QString path = QStringLiteral("/me/partner-posts?") + query.toString();

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            MyPostListResult result;
            if (response.ok) {
                result.success = true;
                result.page = response.json.value(QStringLiteral("page")).toInt();
                result.size = response.json.value(QStringLiteral("size")).toInt();
                result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
                result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();

                const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
                for (const QJsonValue &v : items) {
                    result.items.append(parsePostItem(v.toObject()));
                }
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

void MyPartnerPostApiService::getMyPostDetail(const QString &postId, MyPostDetailCallback callback)
{
    QString path = QStringLiteral("/me/partner-posts/%1").arg(postId);

    client_.getJson(path, tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            MyPostResult result;
            if (response.ok) {
                result.success = true;
                result.post = parsePostItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

void MyPartnerPostApiService::submitReview(const QString &postId, PostActionCallback callback)
{
    QString path = QStringLiteral("/me/partner-posts/%1/submit-review").arg(postId);

    client_.postJson(path, QJsonObject(), tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PostActionResult result;
            if (response.ok) {
                result.success = true;
                result.post = parsePostItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

void MyPartnerPostApiService::withdrawReview(const QString &postId, PostActionCallback callback)
{
    QString path = QStringLiteral("/me/partner-posts/%1/withdraw-review").arg(postId);

    client_.postJson(path, QJsonObject(), tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PostActionResult result;
            if (response.ok) {
                result.success = true;
                result.post = parsePostItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

void MyPartnerPostApiService::unpublish(const QString &postId, PostActionCallback callback)
{
    QString path = QStringLiteral("/me/partner-posts/%1/unpublish").arg(postId);

    client_.postJson(path, QJsonObject(), tokenStore_.accessToken(),
        [callback = std::move(callback)](const ApiClientResponse &response) {
            PostActionResult result;
            if (response.ok) {
                result.success = true;
                result.post = parsePostItem(response.json);
            } else {
                result.success = false;
                result.errorCode = response.error.code;
                result.errorMessage = response.error.message;
                result.errorDetails = response.error.details;
                result.httpStatus = response.error.httpStatus;
            }
            if (callback) callback(result);
        });
}

QJsonObject MyPartnerPostApiService::buildDraftBody(const MyPostDraftRequest &request)
{
    QJsonObject body;
    if (!request.sceneType.isEmpty()) body[QStringLiteral("sceneType")] = request.sceneType;
    if (!request.title.isEmpty()) body[QStringLiteral("title")] = request.title;
    if (!request.description.isEmpty()) body[QStringLiteral("description")] = request.description;
    if (!request.timeMode.isEmpty()) body[QStringLiteral("timeMode")] = request.timeMode;
    if (!request.timeText.isEmpty()) body[QStringLiteral("timeText")] = request.timeText;
    if (!request.startAt.isEmpty()) body[QStringLiteral("startAt")] = request.startAt;
    if (!request.endAt.isEmpty()) body[QStringLiteral("endAt")] = request.endAt;
    if (!request.locationText.isEmpty()) body[QStringLiteral("locationText")] = request.locationText;
    if (request.participantCount > 0) body[QStringLiteral("participantCount")] = request.participantCount;
    if (!request.targetRequirement.isEmpty()) body[QStringLiteral("targetRequirement")] = request.targetRequirement;
    if (!request.contactPreference.isEmpty()) body[QStringLiteral("contactPreference")] = request.contactPreference;

    QJsonArray tagsArray;
    for (const QString &tag : request.tags) {
        tagsArray.append(tag);
    }
    body[QStringLiteral("tags")] = tagsArray;

    QJsonArray attachArray;
    for (const QString &id : request.attachmentIds) {
        attachArray.append(id);
    }
    body[QStringLiteral("attachmentIds")] = attachArray;

    if (!request.scenePayload.isEmpty()) {
        QJsonObject payloadObj;
        for (auto it = request.scenePayload.constBegin(); it != request.scenePayload.constEnd(); ++it) {
            payloadObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        body[QStringLiteral("scenePayload")] = payloadObj;
    }

    return body;
}

MyPostItem MyPartnerPostApiService::parsePostItem(const QJsonObject &obj)
{
    MyPostItem item;
    item.postId = obj.value(QStringLiteral("postId")).toString();
    item.publisherId = obj.value(QStringLiteral("publisherId")).toString();
    item.sceneType = obj.value(QStringLiteral("sceneType")).toString();
    item.status = obj.value(QStringLiteral("status")).toString();
    item.title = obj.value(QStringLiteral("title")).toString();
    item.description = obj.value(QStringLiteral("description")).toString();
    item.timeMode = obj.value(QStringLiteral("timeMode")).toString();
    item.timeText = obj.value(QStringLiteral("timeText")).toString();
    item.startAt = obj.value(QStringLiteral("startAt")).toString();
    item.endAt = obj.value(QStringLiteral("endAt")).toString();
    item.locationText = obj.value(QStringLiteral("locationText")).toString();
    item.participantCount = obj.value(QStringLiteral("participantCount")).toInt();
    item.targetRequirement = obj.value(QStringLiteral("targetRequirement")).toString();
    item.contactPreference = obj.value(QStringLiteral("contactPreference")).toString();
    item.rejectReason = obj.value(QStringLiteral("rejectReason")).toString();
    item.publishedAt = obj.value(QStringLiteral("publishedAt")).toString();
    item.createdAt = obj.value(QStringLiteral("createdAt")).toString();
    item.updatedAt = obj.value(QStringLiteral("updatedAt")).toString();

    const QJsonArray tags = obj.value(QStringLiteral("tags")).toArray();
    for (const QJsonValue &t : tags) {
        item.tags.append(t.toString());
    }

    const QJsonArray attachIds = obj.value(QStringLiteral("attachmentIds")).toArray();
    for (const QJsonValue &a : attachIds) {
        item.attachmentIds.append(a.toString());
    }

    if (obj.contains(QStringLiteral("scenePayload")) && obj.value(QStringLiteral("scenePayload")).isObject()) {
        const QJsonObject payload = obj.value(QStringLiteral("scenePayload")).toObject();
        for (auto it = payload.constBegin(); it != payload.constEnd(); ++it) {
            item.scenePayload.insert(it.key(), it.value().toVariant());
        }
    }

    const QJsonArray actions = obj.value(QStringLiteral("allowedActions")).toArray();
    for (const QJsonValue &a : actions) {
        item.allowedActions.append(a.toString());
    }

    return item;
}
