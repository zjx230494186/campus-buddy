#pragma once

#include <QString>
#include <QStringList>

struct CreateReviewRequest
{
    long long conversationId = 0;
    QString revieweeId;
    int rating = 5;
    QStringList reviewTags;
};

struct UpdateReviewRequest
{
    int rating = 5;
    QStringList reviewTags;
};

struct ReviewItem
{
    long long id = 0;
    long long conversationId = 0;
    QString reviewerId;
    QString revieweeId;
    int rating = 0;
    QStringList reviewTags;
    QString status;
    bool modifiedOnce = false;
    QString createdAt;
    QString updatedAt;
};

struct ReviewResult
{
    bool success = false;
    ReviewItem review;
    QString errorCode;
    QString errorMessage;
};

struct ReviewListResult
{
    bool success = false;
    QList<ReviewItem> items;
    int page = 0;
    int size = 0;
    long long totalElements = 0;
    int totalPages = 0;
    QString errorCode;
    QString errorMessage;
};

struct CreditSummaryTagItem
{
    QString tag;
    int count = 0;
};

struct MyCreditSummaryResult
{
    bool success = false;
    QString userId;
    double averageRating = 0.0;
    long long realConversationCount = 0;
    long long ratingSampleCount = 0;
    QList<CreditSummaryTagItem> topTags;
    int disputedReviewCount = 0;
    QString updatedAt;
    QString errorCode;
    QString errorMessage;
};

struct PublicCreditSummaryResult
{
    bool success = false;
    QString userId;
    double averageRating = 0.0;
    long long realConversationCount = 0;
    long long ratingSampleCount = 0;
    QList<CreditSummaryTagItem> topTags;
    QString updatedAt;
    QString errorCode;
    QString errorMessage;
};
