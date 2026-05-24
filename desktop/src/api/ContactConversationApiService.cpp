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

void ContactConversationApiService::getMyContactCard(ContactCardCallback callback)
{
    QString path = QStringLiteral("/me/contact-card");

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        ContactCardResult result;
        if (response.ok) {
            result.success = true;
            result.hasCard = response.json.value(QStringLiteral("hasCard")).toBool();
            if (result.hasCard) {
                result.wechatId = response.json.value(QStringLiteral("wechatId")).toString();
                result.phoneNumber = response.json.value(QStringLiteral("phoneNumber")).toString();
                result.qqNumber = response.json.value(QStringLiteral("qqNumber")).toString();
                result.updatedAt = response.json.value(QStringLiteral("updatedAt")).toString();
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

void ContactConversationApiService::upsertMyContactCard(const QString &wechatId, const QString &phoneNumber, const QString &qqNumber, ContactCardCallback callback)
{
    QString path = QStringLiteral("/me/contact-card");

    QJsonObject body;
    body[QStringLiteral("wechatId")] = wechatId;
    body[QStringLiteral("phoneNumber")] = phoneNumber;
    body[QStringLiteral("qqNumber")] = qqNumber;

    client_.putJson(path, body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        ContactCardResult result;
        if (response.ok) {
            result.success = true;
            result.hasCard = response.json.value(QStringLiteral("hasCard")).toBool();
            if (result.hasCard) {
                result.wechatId = response.json.value(QStringLiteral("wechatId")).toString();
                result.phoneNumber = response.json.value(QStringLiteral("phoneNumber")).toString();
                result.qqNumber = response.json.value(QStringLiteral("qqNumber")).toString();
                result.updatedAt = response.json.value(QStringLiteral("updatedAt")).toString();
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

void ContactConversationApiService::getContactUnlockStatus(long long conversationId, ContactUnlockStatusCallback callback)
{
    QString path = QStringLiteral("/me/conversations/%1/contact-unlock").arg(conversationId);

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        ContactUnlockStatusResult result;
        if (response.ok) {
            result.success = true;
            result.conversationId = response.json.value(QStringLiteral("conversationId")).toInteger();
            result.status = response.json.value(QStringLiteral("status")).toString();
            result.currentUserConfirmed = response.json.value(QStringLiteral("currentUserConfirmed")).toBool();
            result.peerConfirmed = response.json.value(QStringLiteral("peerConfirmed")).toBool();
            result.currentUserHasContactCard = response.json.value(QStringLiteral("currentUserHasContactCard")).toBool();
            result.peerHasContactCard = response.json.value(QStringLiteral("peerHasContactCard")).toBool();
            result.unlockedAt = response.json.value(QStringLiteral("unlockedAt")).toString();
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

void ContactConversationApiService::confirmContactUnlock(long long conversationId, ContactUnlockStatusCallback callback)
{
    QString path = QStringLiteral("/me/conversations/%1/contact-unlock/confirm").arg(conversationId);
    QJsonObject body;

    client_.postJson(path, body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        ContactUnlockStatusResult result;
        if (response.ok) {
            result.success = true;
            result.conversationId = response.json.value(QStringLiteral("conversationId")).toInteger();
            result.status = response.json.value(QStringLiteral("status")).toString();
            result.currentUserConfirmed = response.json.value(QStringLiteral("currentUserConfirmed")).toBool();
            result.peerConfirmed = response.json.value(QStringLiteral("peerConfirmed")).toBool();
            result.currentUserHasContactCard = response.json.value(QStringLiteral("currentUserHasContactCard")).toBool();
            result.peerHasContactCard = response.json.value(QStringLiteral("peerHasContactCard")).toBool();
            result.unlockedAt = response.json.value(QStringLiteral("unlockedAt")).toString();
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

void ContactConversationApiService::getPeerContactCard(long long conversationId, PeerContactCardCallback callback)
{
    QString path = QStringLiteral("/me/conversations/%1/peer-contact-card").arg(conversationId);

    client_.getJson(path, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        PeerContactCardResult result;
        if (response.ok) {
            result.success = true;
            result.wechatId = response.json.value(QStringLiteral("wechatId")).toString();
            result.phoneNumber = response.json.value(QStringLiteral("phoneNumber")).toString();
            result.qqNumber = response.json.value(QStringLiteral("qqNumber")).toString();
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
