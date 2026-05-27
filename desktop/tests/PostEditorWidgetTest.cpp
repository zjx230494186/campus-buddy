#include <QtTest/QtTest>

#include <QApplication>
#include <QHostAddress>
#include <QLabel>
#include <QPushButton>
#include <QTcpServer>
#include <QTcpSocket>

#include "api/CampusApiClient.h"
#include "api/MyPartnerPostApiService.h"
#include "auth/InMemorySessionTokenStore.h"
#include "domain/ApiClientConfig.h"
#include "ui/PostEditorWidget.h"

class PostEditorWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void editableDraftFromCurrentBackendActionsKeepsControlsEnabled();
    void submitValidationFailureRestoresDraftControls();

private:
    static MyPostItem editableDraft();
    static QUrl serveSingleResponse(const QByteArray &status, const QByteArray &body);
};

MyPostItem PostEditorWidgetTest::editableDraft()
{
    MyPostItem item;
    item.postId = QStringLiteral("post-1");
    item.publisherId = QStringLiteral("user-1");
    item.sceneType = QStringLiteral("INNOVATION_PROJECT");
    item.status = QStringLiteral("DRAFT");
    item.title = QStringLiteral("测试");
    item.description = QStringLiteral("测");
    item.timeMode = QStringLiteral("EXACT_TIME");
    item.timeText = QStringLiteral("测");
    item.locationText = QStringLiteral("测");
    item.participantCount = 2;
    item.targetRequirement = QStringLiteral("测");
    item.contactPreference = QStringLiteral("测");
    item.scenePayload.insert(QStringLiteral("projectDirection"), QStringLiteral("测"));
    item.allowedActions = QStringList{QStringLiteral("EDIT"), QStringLiteral("SUBMIT_REVIEW")};
    return item;
}

QUrl PostEditorWidgetTest::serveSingleResponse(const QByteArray &status, const QByteArray &body)
{
    auto *server = new QTcpServer(qApp);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        server->deleteLater();
        return {};
    }

    QObject::connect(server, &QTcpServer::newConnection, server, [server, status, body]() {
        QTcpSocket *socket = server->nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, server, status, body]() {
            const QByteArray requestData = socket->readAll();
            if (!requestData.contains("\r\n\r\n")) {
                return;
            }

            QByteArray response;
            response.append(status);
            response.append("\r\nContent-Type: application/json");
            response.append("\r\nContent-Length: ");
            response.append(QByteArray::number(body.size()));
            response.append("\r\nConnection: close\r\n\r\n");
            response.append(body);

            socket->write(response);
            socket->disconnectFromHost();
            server->close();
        });
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        QObject::connect(socket, &QTcpSocket::disconnected, server, &QObject::deleteLater);
    });

    return QUrl(QStringLiteral("http://127.0.0.1:%1/api").arg(server->serverPort()));
}

void PostEditorWidgetTest::editableDraftFromCurrentBackendActionsKeepsControlsEnabled()
{
    CampusApiClient client(ApiClientConfig(QStringLiteral("http://127.0.0.1:9/api"), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    MyPartnerPostApiService service(client, tokenStore);
    PostEditorWidget widget(service);

    widget.loadPost(QStringLiteral("post-1"), editableDraft());

    auto *saveButton = widget.findChild<QPushButton *>(QStringLiteral("saveDraftButton"));
    auto *updateButton = widget.findChild<QPushButton *>(QStringLiteral("updateDraftButton"));
    auto *submitButton = widget.findChild<QPushButton *>(QStringLiteral("submitReviewButton"));

    QVERIFY(saveButton);
    QVERIFY(updateButton);
    QVERIFY(submitButton);
    QVERIFY(!saveButton->isEnabled());
    QVERIFY2(updateButton->isEnabled(), "DRAFT posts with backend EDIT action must allow draft updates.");
    QVERIFY(submitButton->isEnabled());
}

void PostEditorWidgetTest::submitValidationFailureRestoresDraftControls()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 400 Bad Request",
        R"({"code":"VALIDATION_FAILED","message":"Validation failed","details":{"title":"is required","scenePayload.projectDirection":"is required for scene INNOVATION_PROJECT"}})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);
    PostEditorWidget widget(service);

    widget.loadPost(QStringLiteral("post-1"), editableDraft());

    auto *updateButton = widget.findChild<QPushButton *>(QStringLiteral("updateDraftButton"));
    auto *submitButton = widget.findChild<QPushButton *>(QStringLiteral("submitReviewButton"));
    auto *statusLabel = widget.findChild<QLabel *>(QStringLiteral("statusLabel"));

    QVERIFY(updateButton);
    QVERIFY(submitButton);
    QVERIFY(statusLabel);
    QVERIFY(updateButton->isEnabled());
    QVERIFY(submitButton->isEnabled());

    QTest::mouseClick(submitButton, Qt::LeftButton);

    QTRY_VERIFY(statusLabel->text().contains(QStringLiteral("校验失败")));
    QVERIFY2(updateButton->isEnabled(), "Validation failure must not leave update draft disabled.");
    QVERIFY2(submitButton->isEnabled(), "Validation failure must restore submit review for the editable draft.");
}

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM")) {
        qputenv("QT_QPA_PLATFORM", "offscreen");
    }
    QApplication app(argc, argv);
    PostEditorWidgetTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "PostEditorWidgetTest.moc"
