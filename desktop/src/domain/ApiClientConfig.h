#pragma once

#include <QString>

class ApiClientConfig
{
public:
    ApiClientConfig();
    ApiClientConfig(QString apiBaseUrl, int requestTimeoutMs, int messagePollIntervalMs, bool technicalSpikeMode);

    QString apiBaseUrl() const;
    int requestTimeoutMs() const;
    int messagePollIntervalMs() const;
    bool technicalSpikeMode() const;

private:
    QString apiBaseUrl_;
    int requestTimeoutMs_;
    int messagePollIntervalMs_;
    bool technicalSpikeMode_;
};
