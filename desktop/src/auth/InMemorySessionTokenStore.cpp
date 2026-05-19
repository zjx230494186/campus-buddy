#include "auth/InMemorySessionTokenStore.h"

QString InMemorySessionTokenStore::accessToken() const
{
    return accessToken_;
}

void InMemorySessionTokenStore::setAccessToken(const QString &token)
{
    accessToken_ = token;
}

void InMemorySessionTokenStore::clear()
{
    accessToken_.clear();
}

bool InMemorySessionTokenStore::hasAccessToken() const
{
    return !accessToken_.isEmpty();
}
