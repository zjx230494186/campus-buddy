#include <QtTest/QtTest>

#include "domain/ApiClientConfig.h"

class ApiClientConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultValuesDescribeTechnicalSpikeRuntime();
};

void ApiClientConfigTest::defaultValuesDescribeTechnicalSpikeRuntime()
{
    const ApiClientConfig config;

    QCOMPARE(config.apiBaseUrl(), QString("http://localhost:8080/api"));
    QCOMPARE(config.requestTimeoutMs(), 10000);
    QCOMPARE(config.messagePollIntervalMs(), 1000);
    QVERIFY(config.technicalSpikeMode());
}

QTEST_MAIN(ApiClientConfigTest)

#include "ApiClientConfigTest.moc"
