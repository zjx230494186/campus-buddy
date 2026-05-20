#include "api/CampusApiClient.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <utility>

bool ApiClientError::hasError() const
{
    return type != None;
}

CampusApiClient::CampusApiClient(ApiClientConfig config, QObject *parent)
    : QObject(parent),
      config_(std::move(config)),
      network_(this)
{
}

void CampusApiClient::setCommonHeaders(QNetworkRequest &request, const QString &accessToken)
{
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Accept", "application/json");
    if (!accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(accessToken).toUtf8());
    }
}

void CampusApiClient::getJson(const QString &path, ResponseCallback callback)
{
    getJson(path, QString(), std::move(callback));
}

void CampusApiClient::getJson(const QString &path, const QString &accessToken, ResponseCallback callback)
{
    QNetworkRequest request(buildUrl(path));
    setCommonHeaders(request, accessToken);

    QNetworkReply *reply = network_.get(request);
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, callback = std::move(callback)]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkErrorText = reply->errorString();
        const QByteArray body = reply->readAll();

        const ApiClientResponse response = parseReply(httpStatus, networkError, networkErrorText, body);
        reply->deleteLater();

        if (callback) {
            callback(response);
        }
    });
}

void CampusApiClient::postJson(const QString &path, const QJsonObject &body, ResponseCallback callback)
{
    postJson(path, body, QString(), std::move(callback));
}

void CampusApiClient::postJson(const QString &path, const QJsonObject &body, const QString &accessToken, ResponseCallback callback)
{
    QNetworkRequest request(buildUrl(path));
    setCommonHeaders(request, accessToken);

    const QByteArray data = QJsonDocument(body).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = network_.post(request, data);
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, callback = std::move(callback)]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkErrorText = reply->errorString();
        const QByteArray body = reply->readAll();

        const ApiClientResponse response = parseReply(httpStatus, networkError, networkErrorText, body);
        reply->deleteLater();

        if (callback) {
            callback(response);
        }
    });
}

void CampusApiClient::deleteResource(const QString &path, const QString &accessToken, ResponseCallback callback)
{
    QNetworkRequest request(buildUrl(path));
    setCommonHeaders(request, accessToken);

    QNetworkReply *reply = network_.deleteResource(request);
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, callback = std::move(callback)]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkErrorText = reply->errorString();
        const QByteArray body = reply->readAll();

        const ApiClientResponse response = parseReply(httpStatus, networkError, networkErrorText, body);
        reply->deleteLater();

        if (callback) {
            callback(response);
        }
    });
}

void CampusApiClient::uploadMultipart(const QString &path, QHttpMultiPart *multiPart, const QString &accessToken, ResponseCallback callback)
{
    QNetworkRequest request(buildUrl(path));
    request.setRawHeader("Accept", "application/json");
    if (!accessToken.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(accessToken).toUtf8());
    }

    QNetworkReply *reply = network_.post(request, multiPart);
    multiPart->setParent(reply);
    QObject::connect(reply, &QNetworkReply::finished, this, [reply, callback = std::move(callback)]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QNetworkReply::NetworkError networkError = reply->error();
        const QString networkErrorText = reply->errorString();
        const QByteArray body = reply->readAll();

        const ApiClientResponse response = parseReply(httpStatus, networkError, networkErrorText, body);
        reply->deleteLater();

        if (callback) {
            callback(response);
        }
    });
}

QUrl CampusApiClient::buildUrl(const QString &path) const
{
    QUrl url(config_.apiBaseUrl());
    QString basePath = url.path();
    QString relativePath = path;

    if (basePath.endsWith('/')) {
        basePath.chop(1);
    }
    while (relativePath.startsWith('/')) {
        relativePath.remove(0, 1);
    }

    url.setPath(basePath + "/" + relativePath);
    return url;
}

ApiClientResponse CampusApiClient::parseReply(int httpStatus, QNetworkReply::NetworkError networkError, const QString &networkErrorText, const QByteArray &body)
{
    ApiClientResponse response;

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    const bool hasJsonObject = parseError.error == QJsonParseError::NoError && document.isObject();

    if (networkError != QNetworkReply::NoError && httpStatus == 0) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = networkErrorText;
        return response;
    }

    if (httpStatus >= 200 && httpStatus < 300 && body.isEmpty()) {
        response.ok = true;
        return response;
    }

    if (!hasJsonObject) {
        response.error.type = ApiClientError::InvalidJson;
        response.error.httpStatus = httpStatus;
        response.error.message = QStringLiteral("Response body is not a JSON object");
        return response;
    }

    const QJsonObject object = document.object();
    if (httpStatus >= 200 && httpStatus < 300) {
        response.ok = true;
        response.json = object;
        return response;
    }

    response.error = parseErrorObject(httpStatus, object);
    return response;
}

ApiClientError CampusApiClient::parseErrorObject(int httpStatus, const QJsonObject &object)
{
    ApiClientError error;
    error.type = ApiClientError::HttpError;
    error.httpStatus = httpStatus;
    error.code = object.value("code").toString();
    error.message = object.value("message").toString();
    error.details = object.value("details").toObject();
    error.traceId = object.value("traceId").toString();
    return error;
}
