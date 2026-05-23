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
