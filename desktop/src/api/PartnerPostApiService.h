#pragma once

#include <functional>

#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/SecureTokenStore.h"
#include "domain/PartnerPostModels.h"

class PartnerPostApiService : public QObject
{
public:
    using PlazaListCallback = std::function<void(const PlazaListResult &)>;
    using PlazaDetailCallback = std::function<void(const PlazaDetailResult &)>;

    explicit PartnerPostApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent = nullptr);

    void listPosts(int page, int size, PlazaListCallback callback);
    void listPosts(const QString &sceneType, const QString &keyword, int page, int size, PlazaListCallback callback);
    void getPostDetail(const QString &postId, PlazaDetailCallback callback);

private:
    static PlazaListItem parseListItem(const QJsonObject &obj);
    static PublicCreditSummary parseCreditSummary(const QJsonObject &obj);
    static PlazaDetailResult parseDetailResponse(const QJsonObject &json);

    CampusApiClient &client_;
    SecureTokenStore &tokenStore_;
};
