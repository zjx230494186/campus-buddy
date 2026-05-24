#include "api/ContactConversationApiService.h"

#include <QJsonArray>
#include <QUrlQuery>

ContactConversationApiService::ContactConversationApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent)
    : QObject(parent),
      client_(client),
      tokenStore_(tokenStore)
{
}

void ContactConversationApiService::requestContact(const QString &postId, const QString &message, ContactRequestCallback callback)
{
    QString path = QStringLiteral("/partner-posts/%1/contact-requests").arg(postId);

    QJsonObject body;
    body[QStringLiteral("message")] = message;

    client_.postJson(path, body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        ContactRequestResult result;
        if (response.ok) {
            result.success = true;
            result.conversationId = response.json.value(QStringLiteral("conversationId")).toInteger();
            result.status = response.json.value(QStringLiteral("status")).toString();
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

void ContactConversationApiService::listConversations(int page, int size, ConversationListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/me/conversations?") + query.toString();

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        ConversationListResult result;
        if (response.ok) {
            result.success = true;
            result.page = response.json.value(QStringLiteral("page")).toInt();
            result.size = response.json.value(QStringLiteral("size")).toInt();
            result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
            result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();

            const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
            for (const QJsonValue &v : items) {
                ConversationListItem item;
                item.conversationId = v.toObject().value(QStringLiteral("conversationId")).toInteger();
                item.status = v.toObject().value(QStringLiteral("status")).toString();
                item.otherParticipantId = v.toObject().value(QStringLiteral("otherParticipantId")).toString();
                item.otherParticipantDisplayName = v.toObject().value(QStringLiteral("otherParticipantDisplayName")).toString();
                item.relatedPostUuid = v.toObject().value(QStringLiteral("relatedPostUuid")).toString();
                item.relatedPostTitle = v.toObject().value(QStringLiteral("relatedPostTitle")).toString();
                item.lastMessagePreview = v.toObject().value(QStringLiteral("lastMessagePreview")).toString();
                item.lastMessageAt = v.toObject().value(QStringLiteral("lastMessageAt")).toString();
                item.updatedAt = v.toObject().value(QStringLiteral("updatedAt")).toString();
                item.unreadCount = v.toObject().value(QStringLiteral("unreadCount")).toInt();
                result.items.append(item);
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

void ContactConversationApiService::sendMessage(long long conversationId, const QString &message, SendMessageCallback callback)
{
    QString path = QStringLiteral("/me/conversations/%1/messages").arg(conversationId);

    QJsonObject body;
    body[QStringLiteral("message")] = message;

    client_.postJson(path, body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        SendMessageResult result;
        if (response.ok) {
            result.success = true;
            result.messageId = response.json.value(QStringLiteral("messageId")).toInteger();
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

void ContactConversationApiService::queryMessages(long long conversationId, int page, int size, MessageListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("page"), QString::number(page));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/me/conversations/%1/messages?").arg(conversationId) + query.toString();

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        MessageListResult result;
        if (response.ok) {
            result.success = true;
            result.page = response.json.value(QStringLiteral("page")).toInt();
            result.size = response.json.value(QStringLiteral("size")).toInt();
            result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
            result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();

            const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
            for (const QJsonValue &v : items) {
                MessageItem item;
                item.messageId = v.toObject().value(QStringLiteral("messageId")).toInteger();
                item.senderId = v.toObject().value(QStringLiteral("senderId")).toString();
                item.messageType = v.toObject().value(QStringLiteral("messageType")).toString();
                item.content = v.toObject().value(QStringLiteral("content")).toString();
                item.createdAt = v.toObject().value(QStringLiteral("createdAt")).toString();
                result.items.append(item);
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

void ContactConversationApiService::queryMessages(long long conversationId, long long afterMessageId, int size, MessageListCallback callback)
{
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("afterMessageId"), QString::number(afterMessageId));
    query.addQueryItem(QStringLiteral("size"), QString::number(size));
    QString path = QStringLiteral("/me/conversations/%1/messages?").arg(conversationId) + query.toString();

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        MessageListResult result;
        if (response.ok) {
            result.success = true;
            result.page = response.json.value(QStringLiteral("page")).toInt();
            result.size = response.json.value(QStringLiteral("size")).toInt();
            result.totalElements = response.json.value(QStringLiteral("totalElements")).toInteger();
            result.totalPages = response.json.value(QStringLiteral("totalPages")).toInt();

            const QJsonArray items = response.json.value(QStringLiteral("items")).toArray();
            for (const QJsonValue &v : items) {
                MessageItem item;
                item.messageId = v.toObject().value(QStringLiteral("messageId")).toInteger();
                item.senderId = v.toObject().value(QStringLiteral("senderId")).toString();
                item.messageType = v.toObject().value(QStringLiteral("messageType")).toString();
                item.content = v.toObject().value(QStringLiteral("content")).toString();
                item.createdAt = v.toObject().value(QStringLiteral("createdAt")).toString();
                result.items.append(item);
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

void ContactConversationApiService::closeConversation(long long conversationId, CloseConversationCallback callback)
{
    QString path = QStringLiteral("/me/conversations/%1/close").arg(conversationId);
    QJsonObject body;

    client_.postJson(path, body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        CloseConversationResult result;
        if (response.ok) {
            result.success = true;
            result.conversationId = response.json.value(QStringLiteral("conversationId")).toInteger();
            result.status = response.json.value(QStringLiteral("status")).toString();
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

void ContactConversationApiService::markConversationRead(long long conversationId, MarkReadCallback callback)
{
    QString path = QStringLiteral("/me/conversations/%1/read").arg(conversationId);
    QJsonObject body;

    client_.postJson(path, body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        MarkReadResult result;
        if (response.ok) {
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
