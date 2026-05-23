#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "api/CampusApiClient.h"
#include "api/PartnerPostApiService.h"
#include "auth/InMemorySessionTokenStore.h"
#include "domain/ApiClientConfig.h"

class PartnerPostApiServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void plazaListUsesCorrectPath();
    void plazaListUsesBearerToken();
    void plazaListParsesPublisherCreditSummary();
    void plazaDetailParsesOwnPostAndCreditSummary();
    void errorResponseConvertsToServiceResult();

private:
    static QUrl serveSingleResponse(const QByteArray &statusLine, const QByteArray &body);
};

QUrl PartnerPostApiServiceTest::serveSingleResponse(const QByteArray &statusLine, const QByteArray &body)
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

void PartnerPostApiServiceTest::plazaListUsesCorrectPath()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    PartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PlazaListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listPosts(QStringLiteral("STUDY"), QStringLiteral("math"), 0, 20, [&](const PlazaListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.page, 0);
    QCOMPARE(result.size, 20);
}

void PartnerPostApiServiceTest::plazaListUsesBearerToken()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("my-bearer-token"));
    PartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PlazaListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listPosts(0, 20, [&](const PlazaListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
}

void PartnerPostApiServiceTest::plazaListParsesPublisherCreditSummary()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"items":[{"postId":"p1","publisherId":"u1","publisherDisplayName":"User1","publisherAuthenticationStatus":"VERIFIED","publisherCreditSummary":{"averageRating":4.5,"ratingSampleCount":10,"realConversationCount":3,"topTags":["math","study"],"updatedAt":"2026-05-23T00:00:00Z"},"sceneType":"STUDY","status":"PUBLISHED","title":"Test Post","description":"desc","tags":["math"],"timeText":"any","locationText":"campus","publishedAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","ownPost":false}],"page":0,"size":20,"totalElements":1,"totalPages":1})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    PartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PlazaListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listPosts(0, 20, [&](const PlazaListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(result.items.size(), 1);
    QCOMPARE(result.items[0].publisherCreditSummary.averageRating, 4.5);
    QCOMPARE(result.items[0].publisherCreditSummary.ratingSampleCount, 10);
    QCOMPARE(result.items[0].publisherCreditSummary.realConversationCount, 3);
    QCOMPARE(result.items[0].publisherCreditSummary.topTags.size(), 2);
    QVERIFY2(!result.items[0].publisherCreditSummary.updatedAt.isEmpty(), "credit summary must have updatedAt");
}

void PartnerPostApiServiceTest::plazaDetailParsesOwnPostAndCreditSummary()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"postId":"p1","publisherId":"u1","publisherDisplayName":"User1","publisherAuthenticationStatus":"VERIFIED","publisherCreditSummary":{"averageRating":3.0,"ratingSampleCount":5,"realConversationCount":1,"topTags":[],"updatedAt":"2026-05-23T00:00:00Z"},"sceneType":"STUDY","status":"PUBLISHED","title":"Detail Post","description":"desc","timeMode":"FLEXIBLE","timeText":"any","startAt":null,"endAt":null,"locationText":"lib","participantCount":2,"targetRequirement":"any","tags":[],"publishedAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","ownPost":true})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    PartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PlazaDetailResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.getPostDetail(QStringLiteral("p1"), [&](const PlazaDetailResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QVERIFY(result.ownPost);
    QCOMPARE(result.publisherAuthenticationStatus, QString("VERIFIED"));
    QCOMPARE(result.publisherCreditSummary.averageRating, 3.0);
}

void PartnerPostApiServiceTest::errorResponseConvertsToServiceResult()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 404 Not Found",
        R"({"code":"POST_NOT_FOUND","message":"Post not found","details":"postId does not exist","traceId":"trace-1"})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    PartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PlazaDetailResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.getPostDetail(QStringLiteral("nonexistent"), [&](const PlazaDetailResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(!result.success);
    QCOMPARE(result.errorCode, QString("POST_NOT_FOUND"));
    QCOMPARE(result.errorMessage, QString("Post not found"));
}

QTEST_MAIN(PartnerPostApiServiceTest)

#include "PartnerPostApiServiceTest.moc"
