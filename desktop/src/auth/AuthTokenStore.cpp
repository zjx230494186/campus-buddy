#include "auth/AuthTokenStore.h"

AuthTokenStore::AuthTokenStore()
    : settings_(QStringLiteral("CampusBuddy"), QStringLiteral("Auth"))
{
}

QString AuthTokenStore::accessToken() const
{
    return settings_.value(QStringLiteral("accessToken")).toString();
}

void AuthTokenStore::setAccessToken(const QString &token)
{
    settings_.setValue(QStringLiteral("accessToken"), token);
}

void AuthTokenStore::clear()
{
    settings_.remove(QStringLiteral("accessToken"));
}

bool AuthTokenStore::hasAccessToken() const
{
    return settings_.contains(QStringLiteral("accessToken")) && !accessToken().isEmpty();
}
