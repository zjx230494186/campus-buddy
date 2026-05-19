#pragma once

#include <QString>

#include "auth/SecureTokenStore.h"

class InMemorySessionTokenStore : public SecureTokenStore
{
public:
    InMemorySessionTokenStore() = default;

    QString accessToken() const override;
    void setAccessToken(const QString &token) override;
    void clear() override;
    bool hasAccessToken() const override;

private:
    QString accessToken_;
};
