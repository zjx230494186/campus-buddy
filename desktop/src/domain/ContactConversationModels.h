#pragma once

#include <QString>

struct ConversationListItem
{
    long long conversationId = 0;
    QString status;
    QString otherParticipantId;
    QString otherParticipantDisplayName;
    QString relatedPostUuid;
    QString relatedPostTitle;
    QString lastMessagePreview;
    QString lastMessageAt;
    QString updatedAt;
    int unreadCount = 0;
};

struct ConversationListResult
{
    bool success = false;
    QList<ConversationListItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
};

struct ContactRequestResult
{
    bool success = false;
    long long conversationId = 0;
    QString status;
    QString errorCode;
    QString errorMessage;
};

struct MessageItem
{
    long long messageId = 0;
    QString senderId;
    QString messageType;
    QString content;
    QString createdAt;
};

struct MessageListResult
{
    bool success = false;
    QList<MessageItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
};

struct SendMessageResult
{
    bool success = false;
    long long messageId = 0;
    QString errorCode;
    QString errorMessage;
};

struct CloseConversationResult
{
    bool success = false;
    long long conversationId = 0;
    QString status;
    QString errorCode;
    QString errorMessage;
};

struct MarkReadResult
{
    bool success = false;
    QString errorCode;
    QString errorMessage;
};

struct ContactCardResult
{
    bool success = false;
    bool hasCard = false;
    QString wechatId;
    QString phoneNumber;
    QString qqNumber;
    QString updatedAt;
    QString errorCode;
    QString errorMessage;
};

struct ContactUnlockStatusResult
{
    bool success = false;
    long long conversationId = 0;
    QString status;
    bool currentUserConfirmed = false;
    bool peerConfirmed = false;
    bool currentUserHasContactCard = false;
    bool peerHasContactCard = false;
    QString unlockedAt;
    QString errorCode;
    QString errorMessage;
};

struct PeerContactCardResult
{
    bool success = false;
    QString wechatId;
    QString phoneNumber;
    QString qqNumber;
    QString errorCode;
    QString errorMessage;
};
