#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "api/CampusApiClient.h"
#include "api/ContactConversationApiService.h"
#include "auth/InMemorySessionTokenStore.h"
#include "domain/ApiClientConfig.h"

class ContactConversationApiServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void requestContactPathAndBody();
    void listConversationsUsesBearerToken();
    void sendMessagePathAndBody();
    void queryMessagesSupportsAfterMessageIdAndSize();
    void conversationListParsesFields();
    void messageListParsesFields();

private:
    struct RawRequest {
        QByteArray headers;
        QByteArray body;
    };

    static QUrl serveSingleResponse(const QByteArray &statusLine, const QByteArray &body);
    static QUrl serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody);
};

QUrl ContactConversationApiServiceTest::serveSingleResponse(const QByteArray &statusLine, const QByteArray &body)
{
    auto *server = new QTcpServer(qApp);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        server->deleteLater();
        return {};
    }

    QObject::connect(server, &QTcpServer::newConnection, server, [server, statusLine, body]() {
        QTcpSocket *socket = server->nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, statusLine, body]() {
            const QByteArray request = socket->readAll();
            if (!request.contains("\r\n\r\n")) return;
            QByteArray response;
            response.append(statusLine);
            response.append("\r\nContent-Type: application/json");
            response.append("\r\nContent-Length: ");
            response.append(QByteArray::number(body.size()));
            response.append("\r\nConnection: close\r\n\r\n");
            response.append(body);
            socket->write(response);
            socket->disconnectFromHost();
        });
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        QObject::connect(socket, &QTcpSocket::disconnected, server, &QObject::deleteLater);
    });
    return QUrl(QString("http://127.0.0.1:%1/api").arg(server->serverPort()));
}

QUrl ContactConversationApiServiceTest::serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody)
{
    auto *server = new QTcpServer(qApp);
    if (!server->listen(QHostAddress::LocalHost, 0)) {
        server->deleteLater();
        return {};
    }

    QObject::connect(server, &QTcpServer::newConnection, server, [server, &captured, responseStatus, responseBody]() {
        QTcpSocket *socket = server->nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, &captured, responseStatus, responseBody]() {
            const QByteArray requestData = socket->readAll();
            const int headerEnd = requestData.indexOf("\r\n\r\n");
            if (headerEnd < 0) return;
            captured.headers = requestData.left(headerEnd);
            captured.body = requestData.mid(headerEnd + 4);
            QByteArray response;
            response.append(responseStatus);
            response.append("\r\nContent-Type: application/json");
            response.append("\r\nContent-Length: ");
            response.append(QByteArray::number(responseBody.size()));
            response.append("\r\nConnection: close\r\n\r\n");
            response.append(responseBody);
            socket->write(response);
            socket->disconnectFromHost();
        });
        QObject::connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        QObject::connect(socket, &QTcpSocket::disconnected, server, &QObject::deleteLater);
    });
    return QUrl(QString("http://127.0.0.1:%1/api").arg(server->serverPort()));
}

void ContactConversationApiServiceTest::requestContactPathAndBody()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"conversationId":42,"status":"ACTIVE"})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ContactConversationApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ContactRequestResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.requestContact(QStringLiteral("abc-123"), QStringLiteral("hi"), [&](const ContactRequestResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.conversationId, 42);

    const QString headerStr = QString::fromUtf8(captured.headers);
    QVERIFY2(headerStr.contains("/partner-posts/abc-123/contact-requests"), "path must include postId");
    QVERIFY2(headerStr.contains("Authorization: Bearer test-token"), "must use Bearer token");

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value("message").toString(), QString("hi"));
}

void ContactConversationApiServiceTest::listConversationsUsesBearerToken()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("my-token"));
    ContactConversationApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ConversationListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listConversations(0, 20, [&](const ConversationListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
}

void ContactConversationApiServiceTest::sendMessagePathAndBody()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"messageId":99})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ContactConversationApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    SendMessageResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.sendMessage(42, QStringLiteral("hello"), [&](const SendMessageResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.messageId, 99);

    const QString headerStr = QString::fromUtf8(captured.headers);
    QVERIFY2(headerStr.contains("/me/conversations/42/messages"), "path must include conversationId");

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value("message").toString(), QString("hello"));
}

void ContactConversationApiServiceTest::queryMessagesSupportsAfterMessageIdAndSize()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK",
        R"({"items":[{"messageId":10,"senderId":"u1","messageType":"USER_TEXT","content":"hi","createdAt":"2026-05-23T00:00:00Z"}],"page":0,"size":1,"totalElements":3,"totalPages":3})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ContactConversationApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MessageListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.queryMessages(42, 5, 1, [&](const MessageListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.items.size(), 1);
    QCOMPARE(result.items[0].messageId, 10);
}

void ContactConversationApiServiceTest::conversationListParsesFields()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK",
        R"({"items":[{"conversationId":1,"status":"ACTIVE","otherParticipantId":"uid-abc","otherParticipantDisplayName":"Alice","relatedPostUuid":"post-uuid","relatedPostTitle":"Study Group","lastMessagePreview":"hello","lastMessageAt":"2026-05-23T01:00:00Z","updatedAt":"2026-05-23T01:00:00Z"}],"page":0,"size":20,"totalElements":1,"totalPages":1})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ContactConversationApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ConversationListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listConversations(0, 20, [&](const ConversationListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.items.size(), 1);
    QCOMPARE(result.items[0].conversationId, 1);
    QCOMPARE(result.items[0].otherParticipantDisplayName, QString("Alice"));
    QCOMPARE(result.items[0].relatedPostUuid, QString("post-uuid"));
    QCOMPARE(result.items[0].lastMessagePreview, QString("hello"));
    QVERIFY2(!result.items[0].updatedAt.isEmpty(), "updatedAt must be present");
}

void ContactConversationApiServiceTest::messageListParsesFields()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK",
        R"({"items":[{"messageId":1,"senderId":"u1","messageType":"USER_TEXT","content":"test msg","createdAt":"2026-05-23T00:00:00Z"}],"page":0,"size":20,"totalElements":1,"totalPages":1})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ContactConversationApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MessageListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.queryMessages(1, 0, 20, [&](const MessageListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.items.size(), 1);
    QCOMPARE(result.items[0].messageId, 1);
    QCOMPARE(result.items[0].senderId, QString("u1"));
    QCOMPARE(result.items[0].messageType, QString("USER_TEXT"));
    QCOMPARE(result.items[0].content, QString("test msg"));
    QVERIFY2(!result.items[0].createdAt.isEmpty(), "createdAt must be present");
}

QTEST_MAIN(ContactConversationApiServiceTest)

#include "ContactConversationApiServiceTest.moc"
