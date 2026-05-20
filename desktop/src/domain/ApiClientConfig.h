#pragma once

#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

class ApiClientConfig
{
public:
    ApiClientConfig();
    ApiClientConfig(QString apiBaseUrl, int requestTimeoutMs, int messagePollIntervalMs, bool technicalSpikeMode);

    static ApiClientConfig fromRuntime(const QStringList &arguments, const QProcessEnvironment &environment);

    QString apiBaseUrl() const;
    int requestTimeoutMs() const;
    int messagePollIntervalMs() const;
    bool technicalSpikeMode() const;

private:
    static QString resolveApiBaseUrl(const QStringList &arguments, const QProcessEnvironment &environment);
    static QString normalizeBaseUrl(const QString &url);

    QString apiBaseUrl_;
    int requestTimeoutMs_;
    int messagePollIntervalMs_;
    bool technicalSpikeMode_;
};
