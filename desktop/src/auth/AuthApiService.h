#pragma once

#include <functional>

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/AuthTokenStore.h"

struct AuthResult
{
    bool success = false;
    QString accessToken;
    QString errorCode;
    QString errorMessage;
};

class AuthApiService : public QObject
{
public:
    using AuthCallback = std::function<void(const AuthResult &)>;

    explicit AuthApiService(CampusApiClient &client, AuthTokenStore &tokenStore, QObject *parent = nullptr);

    void login(const QString &campusEmail, const QString &password, AuthCallback callback);
    void registerAccount(const QString &realName, const QString &studentNumber, const QString &campusEmail, const QString &password, const QString &verificationCode, AuthCallback callback);
    void sendVerificationCode(const QString &campusEmail, AuthCallback callback);
    void verifyCampusEmail(const QString &campusEmail, const QString &code, AuthCallback callback);
    void submitIdentityVerification(const QString &realName, const QString &studentNumber, AuthCallback callback);
    void getIdentityVerificationStatus(AuthCallback callback);

    bool isLoggedIn() const;

private:
    CampusApiClient &client_;
    AuthTokenStore &tokenStore_;
};
