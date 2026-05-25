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
    void conversationsWidgetSourceDoesNotContainHardcodedCredentials();
    void coreDemoUiUsesSharedVisualHelpers();
    void secondBatchUiUsesSharedVisualHelpersAndIdentitySelection();
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
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PlazaWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ConversationsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ReviewCreditWidget.cpp")
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

void StudentPostPlazaWidgetTest::conversationsWidgetSourceDoesNotContainHardcodedCredentials()
{
    QFile file(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ConversationsWidget.cpp"));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());

    QRegularExpression passwordPattern("(password|Password|secret)\\s*[=\"]\\s*\"[^\"]+\"");
    QVERIFY2(!content.contains(passwordPattern), "ConversationsWidget must not contain hardcoded password");
    QVERIFY2(!content.contains("@campus.edu.cn"), "ConversationsWidget must not contain hardcoded email");
}

void StudentPostPlazaWidgetTest::coreDemoUiUsesSharedVisualHelpers()
{
    QFile cmakeFile(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/CMakeLists.txt"));
    QVERIFY2(cmakeFile.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open CMakeLists.txt");
    const QString cmake = QString::fromUtf8(cmakeFile.readAll());
    QVERIFY2(cmake.contains("src/ui/AppStyles.cpp"), "AppStyles.cpp must be built into the desktop app");
    QVERIFY2(cmake.contains("src/ui/UiHelpers.cpp"), "UiHelpers.cpp must be built into the desktop app");

    QFile mainFile(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/main.cpp"));
    QVERIFY2(mainFile.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open main.cpp");
    const QString mainContent = QString::fromUtf8(mainFile.readAll());
    QVERIFY2(mainContent.contains("AppStyles::apply"), "main.cpp must apply the shared desktop style sheet");

    const QStringList coreDemoFiles = {
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/HomePageWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PostEditorWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PlazaWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ConversationsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ReviewCreditWidget.cpp")
    };
    for (const QString &path : coreDemoFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(content.contains("UiHelpers"), qPrintable(path + " must use shared UI helpers"));
    }
}

void StudentPostPlazaWidgetTest::secondBatchUiUsesSharedVisualHelpersAndIdentitySelection()
{
    const QStringList secondBatchFiles = {
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/IdentityVerificationWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/MyPostsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/AdminReviewWidget.cpp")
    };

    for (const QString &path : secondBatchFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(content.contains("UiHelpers"), qPrintable(path + " must use shared UI helpers"));
    }

    QFile adminHeader(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/AdminReviewWidget.h"));
    QVERIFY2(adminHeader.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open AdminReviewWidget.h");
    const QString headerContent = QString::fromUtf8(adminHeader.readAll());
    QVERIFY2(headerContent.contains("onIdentityQueueItemClicked"),
             "AdminReviewWidget must expose an identity queue selection handler");
    QVERIFY2(headerContent.contains("identityQueueItems_"),
             "AdminReviewWidget must retain identity queue items for selected submission IDs");

    QFile adminSource(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/AdminReviewWidget.cpp"));
    QVERIFY2(adminSource.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open AdminReviewWidget.cpp");
    const QString sourceContent = QString::fromUtf8(adminSource.readAll());
    QVERIFY2(sourceContent.contains("connect(identityQueueList_"),
             "AdminReviewWidget must connect identity queue clicks to selection handling");
}

QTEST_MAIN(StudentPostPlazaWidgetTest)

#include "StudentPostPlazaWidgetTest.moc"
