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
    void thirdBatchUsesBusyAndEmptyStateHelpers();
    void fourthBatchUsesFieldErrorsAndReviewBusyStates();
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

void StudentPostPlazaWidgetTest::thirdBatchUsesBusyAndEmptyStateHelpers()
{
    QFile helpersHeader(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/UiHelpers.h"));
    QVERIFY2(helpersHeader.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open UiHelpers.h");
    const QString helpersHeaderContent = QString::fromUtf8(helpersHeader.readAll());
    QVERIFY2(helpersHeaderContent.contains("setButtonBusy"),
             "UiHelpers must expose a shared button busy-state helper");
    QVERIFY2(helpersHeaderContent.contains("emptyStateText"),
             "UiHelpers must expose a shared empty-state text helper");

    const QStringList busyFiles = {
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/IdentityVerificationWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/MyPostsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PlazaWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ConversationsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/AdminReviewWidget.cpp")
    };
    for (const QString &path : busyFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(content.contains("setButtonBusy"), qPrintable(path + " must use shared busy-state helper"));
    }

    const QStringList emptyStateFiles = {
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/MyPostsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PlazaWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ConversationsWidget.cpp"),
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/AdminReviewWidget.cpp")
    };
    for (const QString &path : emptyStateFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(content.contains("emptyStateText"), qPrintable(path + " must use shared empty-state helper"));
    }
}

void StudentPostPlazaWidgetTest::fourthBatchUsesFieldErrorsAndReviewBusyStates()
{
    QFile postEditor(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/PostEditorWidget.cpp"));
    QVERIFY2(postEditor.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open PostEditorWidget.cpp");
    const QString postEditorContent = QString::fromUtf8(postEditor.readAll());
    QVERIFY2(postEditorContent.contains("applyFieldErrors"),
             "PostEditorWidget must map validation errors to field-level helper labels");
    QVERIFY2(postEditorContent.contains("clearFieldErrors"),
             "PostEditorWidget must clear field-level validation messages before new requests");
    QVERIFY2(postEditorContent.contains("setButtonBusy"),
             "PostEditorWidget must use shared busy-state helper for draft and submit actions");

    QFile appStyles(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/AppStyles.cpp"));
    QVERIFY2(appStyles.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open AppStyles.cpp");
    const QString appStylesContent = QString::fromUtf8(appStyles.readAll());
    QVERIFY2(appStylesContent.contains("QLabel[error=\"true\"]"),
             "AppStyles must style field-level error labels");

    QFile reviewCredit(QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ReviewCreditWidget.cpp"));
    QVERIFY2(reviewCredit.open(QIODevice::ReadOnly | QIODevice::Text), "Cannot open ReviewCreditWidget.cpp");
    const QString reviewCreditContent = QString::fromUtf8(reviewCredit.readAll());
    QVERIFY2(reviewCreditContent.contains("setButtonBusy"),
             "ReviewCreditWidget must use shared busy-state helper for request buttons");
    QVERIFY2(reviewCreditContent.contains("emptyStateText"),
             "ReviewCreditWidget must use shared empty-state helper for review lists");
}

QTEST_MAIN(StudentPostPlazaWidgetTest)

#include "StudentPostPlazaWidgetTest.moc"
