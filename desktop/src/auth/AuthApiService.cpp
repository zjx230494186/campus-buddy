#include "auth/AuthApiService.h"

#include <QHttpMultiPart>
#include <QJsonArray>
#include <QNetworkReply>

AuthApiService::AuthApiService(CampusApiClient &client, SecureTokenStore &tokenStore, QObject *parent)
    : QObject(parent),
      client_(client),
      tokenStore_(tokenStore)
{
}

void AuthApiService::login(const QString &campusEmail, const QString &password, AuthCallback callback)
{
    QJsonObject body;
    body[QStringLiteral("campusEmail")] = campusEmail;
    body[QStringLiteral("password")] = password;

    client_.postJson(QStringLiteral("/auth/login"), body, [this, callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
            result.accessToken = response.json.value(QStringLiteral("accessToken")).toString();
            tokenStore_.setAccessToken(result.accessToken);
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void AuthApiService::sendVerificationCode(const QString &campusEmail, AuthCallback callback)
{
    QJsonObject body;
    body[QStringLiteral("campusEmail")] = campusEmail;
    body[QStringLiteral("purpose")] = QStringLiteral("REGISTER_OR_LOGIN");

    client_.postJson(QStringLiteral("/auth/campus-email/verification-codes"), body, [callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void AuthApiService::verifyCampusEmail(const QString &campusEmail, const QString &code, AuthCallback callback)
{
    QJsonObject body;
    body[QStringLiteral("campusEmail")] = campusEmail;
    body[QStringLiteral("code")] = code;
    body[QStringLiteral("purpose")] = QStringLiteral("REGISTER_OR_LOGIN");

    client_.postJson(QStringLiteral("/auth/campus-email/verifications"), body, [callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
            result.verificationTicket = response.json.value(QStringLiteral("verificationTicket")).toString();
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void AuthApiService::registerAccount(const QString &campusEmail, const QString &verificationTicket, const QString &password, const QString &displayName, AuthCallback callback)
{
    QJsonObject body;
    body[QStringLiteral("campusEmail")] = campusEmail;
    body[QStringLiteral("verificationTicket")] = verificationTicket;
    body[QStringLiteral("password")] = password;
    body[QStringLiteral("displayName")] = displayName;

    client_.postJson(QStringLiteral("/auth/register"), body, [callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void AuthApiService::uploadIdentityMaterial(const QByteArray &fileData, const QString &fileName, const QString &contentType, AuthCallback callback)
{
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"%1\"").arg(fileName)));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));
    filePart.setBody(fileData);
    multiPart->append(filePart);

    client_.uploadMultipart(QStringLiteral("/auth/identity-verifications/materials"), multiPart, tokenStore_.accessToken(),
                            [callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
            result.attachmentId = response.json.value(QStringLiteral("attachmentId")).toString();
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void AuthApiService::submitIdentityVerification(const QString &realName, const QString &studentNumber, const QString &college, const QString &major, const QString &grade, const QString &materialAttachmentId, AuthCallback callback)
{
    QJsonObject body;
    body[QStringLiteral("realName")] = realName;
    body[QStringLiteral("studentNumber")] = studentNumber;
    body[QStringLiteral("college")] = college;
    body[QStringLiteral("major")] = major;
    body[QStringLiteral("grade")] = grade;
    body[QStringLiteral("materialAttachmentId")] = materialAttachmentId;

    client_.postJson(QStringLiteral("/auth/identity-verifications"), body, tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
            result.authenticationStatus = response.json.value(QStringLiteral("authenticationStatus")).toString();
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

void AuthApiService::getIdentityVerificationStatus(AuthCallback callback)
{
    client_.getJson(QStringLiteral("/auth/identity-verifications/me"), tokenStore_.accessToken(), [callback = std::move(callback)](const ApiClientResponse &response) {
        AuthResult result;
        if (response.ok) {
            result.success = true;
            result.authenticationStatus = response.json.value(QStringLiteral("authenticationStatus")).toString();
            result.reviewStatus = response.json.value(QStringLiteral("reviewStatus")).toString();
            result.rejectReason = response.json.value(QStringLiteral("rejectReason")).toString();
            const QJsonArray actions = response.json.value(QStringLiteral("allowedActions")).toArray();
            for (const QJsonValue &v : actions) {
                result.allowedActions.append(v.toString());
            }
        } else {
            result.success = false;
            result.errorCode = response.error.code;
            result.errorMessage = response.error.message;
        }
        if (callback) {
            callback(result);
        }
    });
}

bool AuthApiService::isLoggedIn() const
{
    return tokenStore_.hasAccessToken();
}
