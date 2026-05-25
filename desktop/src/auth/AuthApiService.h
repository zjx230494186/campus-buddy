#pragma once

#include <functional>

#include <QJsonObject>
#include <QObject>
#include <QString>

#include "api/CampusApiClient.h"
#include "auth/SecureTokenStore.h"

struct AuthResult
{
    bool success = false;
    QString accessToken;
    QString authenticationStatus;
    QString verificationTicket;
    QString attachmentId;
    QString reviewStatus;
    QString rejectReason;
    QStringList allowedActions;
    QString accountRole;
    QString displayName;
    QString errorCode;
    QString errorMessage;
};

class AuthApiService : public QObject
{
public:
    using AuthCallback = std::function<void(const AuthResult &)>;

    explicit AuthApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent = nullptr);

    void login(const QString &campusEmail, const QString &password, AuthCallback callback);
    void sendVerificationCode(const QString &campusEmail, AuthCallback callback);
    void verifyCampusEmail(const QString &campusEmail, const QString &code, AuthCallback callback);
    void registerAccount(const QString &campusEmail, const QString &verificationTicket, const QString &password, const QString &displayName, AuthCallback callback);
    void uploadIdentityMaterial(const QByteArray &fileData, const QString &fileName, const QString &contentType, AuthCallback callback);
    void submitIdentityVerification(const QString &realName, const QString &studentNumber, const QString &college, const QString &major, const QString &grade, const QString &materialAttachmentId, AuthCallback callback);
    void getIdentityVerificationStatus(AuthCallback callback);

    bool isLoggedIn() const;

private:
    CampusApiClient &client_;
    SecureTokenStore &tokenStore_;
};
