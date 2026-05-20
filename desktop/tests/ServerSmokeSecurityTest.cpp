#include <QtTest/QtTest>

#include <QFile>
#include <QRegularExpression>

class ServerSmokeSecurityTest : public QObject
{
    Q_OBJECT

private slots:
    void smokeDoesNotContainHardcodedPassword();
    void smokeDoesNotContainHardcodedEmail();
    void smokeRequiresEnvVariables();
};

void ServerSmokeSecurityTest::smokeDoesNotContainHardcodedPassword()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/tests/QtServerIntegrationSmoke.cpp"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open QtServerIntegrationSmoke.cpp");
    const QString content = QString::fromUtf8(file.readAll());

    QRegularExpression passwordPattern("(password|Password|passwd|secret)\\s*[=\"]\\s*\"[^\"]+\"");
    QVERIFY2(!content.contains(passwordPattern),
             "QtServerIntegrationSmoke.cpp must not contain hardcoded password strings");
}

void ServerSmokeSecurityTest::smokeDoesNotContainHardcodedEmail()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/tests/QtServerIntegrationSmoke.cpp"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open QtServerIntegrationSmoke.cpp");
    const QString content = QString::fromUtf8(file.readAll());

    QVERIFY2(!content.contains("@campus.edu.cn") && !content.contains("@edu.cn"),
             "QtServerIntegrationSmoke.cpp must not contain hardcoded email addresses");
}

void ServerSmokeSecurityTest::smokeRequiresEnvVariables()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/tests/QtServerIntegrationSmoke.cpp"));
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open QtServerIntegrationSmoke.cpp");
    const QString content = QString::fromUtf8(file.readAll());

    QVERIFY2(content.contains("CAMPUS_BUDDY_SMOKE_EMAIL"),
             "QtServerIntegrationSmoke.cpp must reference CAMPUS_BUDDY_SMOKE_EMAIL");
    QVERIFY2(content.contains("CAMPUS_BUDDY_SMOKE_PASSWORD"),
             "QtServerIntegrationSmoke.cpp must reference CAMPUS_BUDDY_SMOKE_PASSWORD");
}

QTEST_MAIN(ServerSmokeSecurityTest)

#include "ServerSmokeSecurityTest.moc"
