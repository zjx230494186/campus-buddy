#include "domain/ApiClientConfig.h"

#include <utility>

ApiClientConfig::ApiClientConfig()
    : apiBaseUrl_("http://localhost:8080/api"),
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
