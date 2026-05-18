#include <QtTest/QtTest>

#include <QSettings>

#include "auth/AuthTokenStore.h"

class AuthTokenStoreTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void defaultStoreIsEmpty();
    void setAndRetrieveAccessToken();
    void clearRemovesAccessToken();
    void hasAccessTokenReturnsFalseAfterClear();
    void cleanupTestCase();

private:
    static constexpr const char *TEST_KEY = "accessToken";
};

void AuthTokenStoreTest::initTestCase()
{
    QSettings settings(QStringLiteral("CampusBuddy"), QStringLiteral("Auth"));
    settings.remove(TEST_KEY);
}

void AuthTokenStoreTest::defaultStoreIsEmpty()
{
    QSettings settings(QStringLiteral("CampusBuddy"), QStringLiteral("Auth"));
    settings.remove(TEST_KEY);

    AuthTokenStore store;
    QVERIFY(!store.hasAccessToken());
    QVERIFY(store.accessToken().isEmpty());
}

void AuthTokenStoreTest::setAndRetrieveAccessToken()
{
    AuthTokenStore store;
    store.setAccessToken(QStringLiteral("test-jwt-token-123"));
    QVERIFY(store.hasAccessToken());
    QCOMPARE(store.accessToken(), QString("test-jwt-token-123"));

    store.clear();
}

void AuthTokenStoreTest::clearRemovesAccessToken()
{
    AuthTokenStore store;
    store.setAccessToken(QStringLiteral("to-be-cleared"));
    store.clear();
    QVERIFY(!store.hasAccessToken());
    QVERIFY(store.accessToken().isEmpty());
}

void AuthTokenStoreTest::hasAccessTokenReturnsFalseAfterClear()
{
    AuthTokenStore store;
    store.setAccessToken(QStringLiteral("temp"));
    QVERIFY(store.hasAccessToken());
    store.clear();
    QVERIFY(!store.hasAccessToken());
}

void AuthTokenStoreTest::cleanupTestCase()
{
    QSettings settings(QStringLiteral("CampusBuddy"), QStringLiteral("Auth"));
    settings.remove(TEST_KEY);
}

QTEST_MAIN(AuthTokenStoreTest)

#include "AuthTokenStoreTest.moc"
