#pragma once

#include <QString>

class SecureTokenStore
{
public:
    virtual ~SecureTokenStore() = default;

    virtual QString accessToken() const = 0;
    virtual void setAccessToken(const QString &token) = 0;
    virtual void clear() = 0;
    virtual bool hasAccessToken() const = 0;
};
