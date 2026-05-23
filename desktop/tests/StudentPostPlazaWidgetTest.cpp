#include <QtTest/QtTest>

#include <QFile>
#include <QRegularExpression>

class StudentPostPlazaWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void widgetLayerDoesNotDirectlyUseNetworkAccessManager();
    void postEditorWidgetSourceDoesNotContainHardcodedCredentials();
    void myPostsWidgetSourceDoesNotContainHardcodedCredentials();
    void plazaWidgetSourceDoesNotContainHardcodedCredentials();
};

void StudentPostPlazaWidgetTest::widgetLayerDoesNotDirectlyUseNetworkAccessManager()
{
    const QStringList widgetLayerFiles = {
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/main.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/LoginWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/RegisterWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/HomePageWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/IdentityVerificationWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PostEditorWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/MyPostsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PlazaWidget.cpp")
    };

    for (const QString &path : widgetLayerFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(!content.contains("QNetworkAccessManager"),
                 qPrintable(path + " must not directly use QNetworkAccessManager"));
    }
}

void StudentPostPlazaWidgetTest::postEditorWidgetSourceDoesNotContainHardcodedCredentials()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PostEditorWidget.cpp"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());

    QRegularExpression passwordPattern("(password|Password|secret)\\s*[=\"]\\s*\"[^\"]+\"");
    QVERIFY2(!content.contains(passwordPattern), "PostEditorWidget must not contain hardcoded password");
    QVERIFY2(!content.contains("@campus.edu.cn"), "PostEditorWidget must not contain hardcoded email");
}

void StudentPostPlazaWidgetTest::myPostsWidgetSourceDoesNotContainHardcodedCredentials()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/MyPostsWidget.cpp"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());

    QRegularExpression passwordPattern("(password|Password|secret)\\s*[=\"]\\s*\"[^\"]+\"");
    QVERIFY2(!content.contains(passwordPattern), "MyPostsWidget must not contain hardcoded password");
}

void StudentPostPlazaWidgetTest::plazaWidgetSourceDoesNotContainHardcodedCredentials()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PlazaWidget.cpp"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());

    QRegularExpression passwordPattern("(password|Password|secret)\\s*[=\"]\\s*\"[^\"]+\"");
    QVERIFY2(!content.contains(passwordPattern), "PlazaWidget must not contain hardcoded password");
}

QTEST_MAIN(StudentPostPlazaWidgetTest)

#include "StudentPostPlazaWidgetTest.moc"
