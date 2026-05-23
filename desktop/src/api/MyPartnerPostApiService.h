#pragma once

#include <functional>

#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/SecureTokenStore.h"
#include "domain/MyPartnerPostModels.h"

class MyPartnerPostApiService : public QObject
{
public:
    using CreateDraftCallback = std::function<void(const MyPostResult &)>;
    using UpdateDraftCallback = std::function<void(const MyPostResult &)>;
    using MyPostListCallback = std::function<void(const MyPostListResult &)>;
    using MyPostDetailCallback = std::function<void(const MyPostResult &)>;
    using PostActionCallback = std::function<void(const PostActionResult &)>;

    explicit MyPartnerPostApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent = nullptr);

    void createDraft(const MyPostDraftRequest &request, CreateDraftCallback callback);
    void updateDraft(const QString &postId, const MyPostDraftRequest &request, UpdateDraftCallback callback);
    void listMyPosts(int page, int size, MyPostListCallback callback);
    void listMyPosts(const QString &status, int page, int size, MyPostListCallback callback);
    void getMyPostDetail(const QString &postId, MyPostDetailCallback callback);
    void submitReview(const QString &postId, PostActionCallback callback);
    void withdrawReview(const QString &postId, PostActionCallback callback);
    void unpublish(const QString &postId, PostActionCallback callback);

private:
    static QJsonObject buildDraftBody(const MyPostDraftRequest &request);
    static MyPostItem parsePostItem(const QJsonObject &obj);

    CampusApiClient &client_;
    SecureTokenStore &tokenStore_;
};
