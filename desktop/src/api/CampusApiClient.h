#pragma once

#include <functional>

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>

#include "domain/ApiClientConfig.h"

struct ApiClientError
{
    enum Type {
        None,
        NetworkError,
        HttpError,
        InvalidJson
    };

    Type type = None;
    int httpStatus = 0;
    QString code;
    QString message;
    QJsonObject details;
    QString traceId;

    bool hasError() const;
};

struct ApiClientResponse
{
    bool ok = false;
    QJsonObject json;
    ApiClientError error;
};

class CampusApiClient : public QObject
{
public:
    using ResponseCallback = std::function<void(const ApiClientResponse &)>;

    explicit CampusApiClient(ApiClientConfig config, QObject *parent = nullptr);

    void getJson(const QString &path, ResponseCallback callback);
    void getJson(const QString &path, const QString &accessToken, ResponseCallback callback);
    void postJson(const QString &path, const QJsonObject &body, ResponseCallback callback);
    void postJson(const QString &path, const QJsonObject &body, const QString &accessToken, ResponseCallback callback);

private:
    QUrl buildUrl(const QString &path) const;
    static ApiClientResponse parseReply(int httpStatus, QNetworkReply::NetworkError networkError, const QString &networkErrorText, const QByteArray &body);
    static ApiClientError parseErrorObject(int httpStatus, const QJsonObject &object);
    static void setCommonHeaders(QNetworkRequest &request, const QString &accessToken = QString());

    ApiClientConfig config_;
    QNetworkAccessManager network_;
};
