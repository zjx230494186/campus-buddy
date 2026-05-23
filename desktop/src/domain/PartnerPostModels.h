#pragma once

#include <QString>
#include <QStringList>

struct PublicCreditSummary
{
    double averageRating = 0.0;
    int ratingSampleCount = 0;
    int realConversationCount = 0;
    QStringList topTags;
    QString updatedAt;
};

struct PlazaListItem
{
    QString postId;
    QString publisherId;
    QString publisherDisplayName;
    QString publisherAuthenticationStatus;
    PublicCreditSummary publisherCreditSummary;
    QString sceneType;
    QString status;
    QString title;
    QString description;
    QStringList tags;
    QString timeText;
    QString locationText;
    QString publishedAt;
    QString updatedAt;
    bool ownPost = false;
};

struct PlazaListResult
{
    bool success = false;
    QList<PlazaListItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
};

struct PlazaDetailResult
{
    bool success = false;
    QString postId;
    QString publisherId;
    QString publisherDisplayName;
    QString publisherAuthenticationStatus;
    PublicCreditSummary publisherCreditSummary;
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
    QStringList tags;
    QString publishedAt;
    QString updatedAt;
    bool ownPost = false;
    QString errorCode;
    QString errorMessage;
};
