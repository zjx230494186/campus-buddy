#pragma once

#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVariant>

struct MyPostDraftRequest
{
    QString sceneType;
    QString title;
    QString description;
    QString timeMode;
    QString timeText;
    QString startAt;
    QString endAt;
    QString locationText;
    int participantCount = 0;
    QString targetRequirement;
    QString contactPreference;
    QStringList tags;
    QStringList attachmentIds;
    QMap<QString, QVariant> scenePayload;
};

struct MyPostItem
{
    QString postId;
    QString publisherId;
    QString sceneType;
    QString status;
    QString title;
    QString description;
    QString timeMode;
    QString timeText;
    QString startAt;
    QString endAt;
    QString locationText;
    int participantCount = 0;
    QString targetRequirement;
    QString contactPreference;
    QStringList tags;
    QStringList attachmentIds;
    QMap<QString, QVariant> scenePayload;
    QString rejectReason;
    QString publishedAt;
    QString createdAt;
    QString updatedAt;
    QStringList allowedActions;
};

struct MyPostResult
{
    bool success = false;
    MyPostItem post;
    QString errorCode;
    QString errorMessage;
    QJsonObject errorDetails;
    int httpStatus = 0;
};

struct MyPostListResult
{
    bool success = false;
    QList<MyPostItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
    QJsonObject errorDetails;
    int httpStatus = 0;
};

struct PostActionResult
{
    bool success = false;
    MyPostItem post;
    QString errorCode;
    QString errorMessage;
    QJsonObject errorDetails;
    int httpStatus = 0;
};
