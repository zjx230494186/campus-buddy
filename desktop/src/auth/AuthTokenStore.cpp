#include "auth/AuthTokenStore.h"

AuthTokenStore::AuthTokenStore() = default;

QString AuthTokenStore::accessToken() const
{
    return accessToken_;
}

void AuthTokenStore::setAccessToken(const QString &token)
{
    accessToken_ = token;
}

void AuthTokenStore::clear()
{
    accessToken_.clear();
}

bool AuthTokenStore::hasAccessToken() const
{
    return !accessToken_.isEmpty();
}
