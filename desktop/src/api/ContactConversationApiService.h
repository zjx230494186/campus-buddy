#pragma once

#include <functional>

#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/SecureTokenStore.h"
#include "domain/ContactConversationModels.h"

class ContactConversationApiService : public QObject
{
public:
    using ContactRequestCallback = std::function<void(const ContactRequestResult &)>;
    using ConversationListCallback = std::function<void(const ConversationListResult &)>;
    using SendMessageCallback = std::function<void(const SendMessageResult &)>;
    using MessageListCallback = std::function<void(const MessageListResult &)>;
    using CloseConversationCallback = std::function<void(const CloseConversationResult &)>;
    using MarkReadCallback = std::function<void(const MarkReadResult &)>;

    explicit ContactConversationApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent = nullptr);

    void requestContact(const QString &postId, const QString &message, ContactRequestCallback callback);
    void listConversations(int page, int size, ConversationListCallback callback);
    void sendMessage(long long conversationId, const QString &message, SendMessageCallback callback);
    void queryMessages(long long conversationId, int page, int size, MessageListCallback callback);
    void queryMessages(long long conversationId, long long afterMessageId, int size, MessageListCallback callback);
    void closeConversation(long long conversationId, CloseConversationCallback callback);
    void markConversationRead(long long conversationId, MarkReadCallback callback);

private:
    CampusApiClient &client_;
    SecureTokenStore &tokenStore_;
};
