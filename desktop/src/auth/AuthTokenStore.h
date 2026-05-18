#pragma once

#include <QSettings>
#include <QString>

class AuthTokenStore
{
public:
    AuthTokenStore();

    QString accessToken() const;
    void setAccessToken(const QString &token);
    void clear();

    bool hasAccessToken() const;

private:
    QSettings settings_;
};
