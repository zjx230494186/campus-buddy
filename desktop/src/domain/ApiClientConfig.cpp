#include "domain/ApiClientConfig.h"

#include <utility>

namespace {
const QString ENV_KEY = "CAMPUS_BUDDY_API_BASE_URL";
const QString ARG_PREFIX = "--api-base-url=";
const QString DEFAULT_BASE_URL = "http://localhost:8080/api";
}

ApiClientConfig::ApiClientConfig()
    : apiBaseUrl_(DEFAULT_BASE_URL),
      requestTimeoutMs_(10000),
      messagePollIntervalMs_(1000),
      technicalSpikeMode_(true)
{
}

ApiClientConfig::ApiClientConfig(QString apiBaseUrl, int requestTimeoutMs, int messagePollIntervalMs, bool technicalSpikeMode)
    : apiBaseUrl_(std::move(apiBaseUrl)),
      requestTimeoutMs_(requestTimeoutMs),
      messagePollIntervalMs_(messagePollIntervalMs),
      technicalSpikeMode_(technicalSpikeMode)
{
}

ApiClientConfig ApiClientConfig::fromRuntime(const QStringList &arguments, const QProcessEnvironment &environment)
{
    const QString baseUrl = resolveApiBaseUrl(arguments, environment);
    return ApiClientConfig(baseUrl, 10000, 1000, true);
}

QString ApiClientConfig::resolveApiBaseUrl(const QStringList &arguments, const QProcessEnvironment &environment)
{
    QString argValue;
    for (const QString &arg : arguments) {
        if (arg.startsWith(ARG_PREFIX)) {
            argValue = arg.mid(ARG_PREFIX.length());
            break;
        }
    }

    if (!argValue.isEmpty()) {
        return normalizeBaseUrl(argValue);
    }

    const QString envValue = environment.value(ENV_KEY);
    if (!envValue.isEmpty()) {
        return normalizeBaseUrl(envValue);
    }

    return DEFAULT_BASE_URL;
}

QString ApiClientConfig::normalizeBaseUrl(const QString &url)
{
    QString normalized = url;
    while (normalized.endsWith('/')) {
        normalized.chop(1);
    }
    return normalized;
}

QString ApiClientConfig::apiBaseUrl() const
{
    return apiBaseUrl_;
}

int ApiClientConfig::requestTimeoutMs() const
{
    return requestTimeoutMs_;
}

int ApiClientConfig::messagePollIntervalMs() const
{
    return messagePollIntervalMs_;
}

bool ApiClientConfig::technicalSpikeMode() const
{
    return technicalSpikeMode_;
}
