#include <QtTest/QtTest>

#include <QProcessEnvironment>
#include <QStringList>

#include "domain/ApiClientConfig.h"

class ApiClientConfigTest : public QObject
{
    Q_OBJECT

private slots:
    void defaultValuesDescribeTechnicalSpikeRuntime();
    void fromRuntimeUsesDefaultWhenNoEnvOrArg();
    void fromRuntimeReadsEnvironmentVariable();
    void fromRuntimeCommandLineOverridesEnvironment();
    void fromRuntimeTrimsTrailingSlash();
    void fromRuntimeFallsBackToDefaultOnEmptyEnv();
    void fromRuntimeFallsBackToDefaultOnEmptyArg();
};

void ApiClientConfigTest::defaultValuesDescribeTechnicalSpikeRuntime()
{
    const ApiClientConfig config;

    QCOMPARE(config.apiBaseUrl(), QString("http://localhost:8080/api"));
    QCOMPARE(config.requestTimeoutMs(), 10000);
    QCOMPARE(config.messagePollIntervalMs(), 1000);
    QVERIFY(config.technicalSpikeMode());
}

void ApiClientConfigTest::fromRuntimeUsesDefaultWhenNoEnvOrArg()
{
    QStringList args;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.remove("CAMPUS_BUDDY_API_BASE_URL");

    const ApiClientConfig config = ApiClientConfig::fromRuntime(args, env);

    QCOMPARE(config.apiBaseUrl(), QString("http://localhost:8080/api"));
}

void ApiClientConfigTest::fromRuntimeReadsEnvironmentVariable()
{
    QStringList args;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("CAMPUS_BUDDY_API_BASE_URL", "http://114.116.203.78/api");

    const ApiClientConfig config = ApiClientConfig::fromRuntime(args, env);

    QCOMPARE(config.apiBaseUrl(), QString("http://114.116.203.78/api"));
}

void ApiClientConfigTest::fromRuntimeCommandLineOverridesEnvironment()
{
    QStringList args;
    args << "campus-buddy-desktop" << "--api-base-url=http://192.168.1.100/api";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("CAMPUS_BUDDY_API_BASE_URL", "http://114.116.203.78/api");

    const ApiClientConfig config = ApiClientConfig::fromRuntime(args, env);

    QCOMPARE(config.apiBaseUrl(), QString("http://192.168.1.100/api"));
}

void ApiClientConfigTest::fromRuntimeTrimsTrailingSlash()
{
    QStringList args;
    args << "campus-buddy-desktop" << "--api-base-url=http://example.com/api/";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    const ApiClientConfig config = ApiClientConfig::fromRuntime(args, env);

    QCOMPARE(config.apiBaseUrl(), QString("http://example.com/api"));
}

void ApiClientConfigTest::fromRuntimeFallsBackToDefaultOnEmptyEnv()
{
    QStringList args;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("CAMPUS_BUDDY_API_BASE_URL", "");

    const ApiClientConfig config = ApiClientConfig::fromRuntime(args, env);

    QCOMPARE(config.apiBaseUrl(), QString("http://localhost:8080/api"));
}

void ApiClientConfigTest::fromRuntimeFallsBackToDefaultOnEmptyArg()
{
    QStringList args;
    args << "campus-buddy-desktop" << "--api-base-url=";
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    const ApiClientConfig config = ApiClientConfig::fromRuntime(args, env);

    QCOMPARE(config.apiBaseUrl(), QString("http://localhost:8080/api"));
}

QTEST_MAIN(ApiClientConfigTest)

#include "ApiClientConfigTest.moc"
