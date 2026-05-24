#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "api/CampusApiClient.h"
#include "api/AdminReviewApiService.h"
#include "auth/InMemorySessionTokenStore.h"
#include "domain/ApiClientConfig.h"

class AdminReviewApiServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void listPartnerPostReviewQueueUsesCorrectPathAndQuery();
    void getPartnerPostAdminDetailUsesCorrectPath();
    void reviewPartnerPostApproveSendsCorrectBody();
    void reviewPartnerPostRejectSendsReasonInBody();
    void listPendingIdentityVerificationsUsesCorrectPathAndQuery();
    void reviewIdentityVerificationApprovedSendsCorrectBody();
    void reviewIdentityVerificationRejectedSendsRejectReason();
    void errorResponseConvertsToServiceResult();

private:
    struct RawRequest {
        QByteArray headers;
        QByteArray body;
    };

    static QUrl serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody);
    static QByteArray extractMethod(const RawRequest &captured);
    static QByteArray extractPath(const RawRequest &captured);
    static QByteArray extractHeader(const RawRequest &captured, const QByteArray &headerName);
    static QJsonObject extractBodyJson(const RawRequest &captured);
};

QUrl AdminReviewApiServiceTest::serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody)
{
    auto *server = new QTcpServer(qApp);
    if (!server->listen(QHostAddress::LocalHost, 0)) { server->deleteLater(); return {}; }
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
            response.append("\r\nContent-Type: application/json\r\nContent-Length: ");
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

QByteArray AdminReviewApiServiceTest::extractMethod(const RawRequest &captured)
{
    const int space = captured.headers.indexOf(' ');
    return space < 0 ? QByteArray() : captured.headers.left(space);
}

QByteArray AdminReviewApiServiceTest::extractPath(const RawRequest &captured)
{
    const int firstSpace = captured.headers.indexOf(' ');
    if (firstSpace < 0) return {};
    const int secondSpace = captured.headers.indexOf(' ', firstSpace + 1);
    return secondSpace < 0 ? QByteArray() : captured.headers.mid(firstSpace + 1, secondSpace - firstSpace - 1);
}

QByteArray AdminReviewApiServiceTest::extractHeader(const RawRequest &captured, const QByteArray &headerName)
{
    const QList<QByteArray> lines = captured.headers.split('\n');
    for (const QByteArray &line : lines) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.startsWith(headerName + ":")) return trimmed.mid(headerName.size() + 1).trimmed();
    }
    return {};
}

QJsonObject AdminReviewApiServiceTest::extractBodyJson(const RawRequest &captured)
{
    return QJsonDocument::fromJson(captured.body).object();
}

static const char *QUEUE_RESPONSE = R"({"items":[{"postId":"p1","publisherId":"u1","publisherDisplayName":"test","sceneType":"MEAL","status":"PENDING_REVIEW","title":"test post","summary":"s","timeText":"t","locationText":"l","updatedAt":"2026-05-24T00:00:00Z"}],"page":0,"size":20,"totalElements":1,"totalPages":1})";

static const char *DETAIL_RESPONSE = R"({"postId":"p1","publisherId":"u1","publisherDisplayName":"test","publisherAuthenticationStatus":"VERIFIED","sceneType":"MEAL","status":"PENDING_REVIEW","title":"test post","description":"desc","timeMode":"EXACT_TIME","timeText":"12:00","startAt":null,"endAt":null,"locationText":"loc","participantCount":2,"targetRequirement":"req","tags":["tag1"],"scenePayload":"{}","rejectReason":null,"reviewedBy":null,"reviewedAt":null,"publishedAt":null,"createdAt":"2026-05-24T00:00:00Z","updatedAt":"2026-05-24T00:00:00Z"})";

static const char *PENDING_IV_RESPONSE = R"({"items":[{"submissionId":1,"userId":"u1","realName":"张三","studentNumber":"2024001","college":"计算机学院","major":"软件工程","grade":"2024","reviewStatus":"PENDING_REVIEW","submittedAt":"2026-05-24T00:00:00Z","materialAttachmentId":"mat1","materialContentType":"application/pdf","materialSizeBytes":1024}],"page":0,"size":20,"totalElements":1,"totalPages":1})";

void AdminReviewApiServiceTest::listPartnerPostReviewQueueUsesCorrectPathAndQuery()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", QUEUE_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    PartnerPostReviewQueueResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listPartnerPostReviewQueue(0, 20, [&](const PartnerPostReviewQueueResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    const QByteArray path = extractPath(captured);
    QVERIFY2(path.startsWith("/api/admin/partner-posts/review-queue?"), "Path must start with /api/admin/partner-posts/review-queue?");
    QVERIFY2(path.contains("page=0"), "Must contain page=0");
    QVERIFY2(path.contains("size=20"), "Must contain size=20");
    const QByteArray auth = extractHeader(captured, "Authorization");
    QVERIFY2(auth.startsWith("Bearer "), "Must send Bearer token");
    QCOMPARE(result.items.size(), 1);
    QCOMPARE(result.items[0].postId, QString("p1"));
    QCOMPARE(result.items[0].status, QString("PENDING_REVIEW"));
}

void AdminReviewApiServiceTest::getPartnerPostAdminDetailUsesCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DETAIL_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    AdminPostDetailResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.getPartnerPostAdminDetail(QStringLiteral("p1"), [&](const AdminPostDetailResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    QCOMPARE(extractPath(captured), QByteArray("/api/admin/partner-posts/p1"));
    QCOMPARE(result.detail.title, QString("test post"));
    QCOMPARE(result.detail.sceneType, QString("MEAL"));
    QCOMPARE(result.detail.participantCount, 2);
    QCOMPARE(result.detail.tags.size(), 1);
    QCOMPARE(result.detail.tags[0], QString("tag1"));
}

void AdminReviewApiServiceTest::reviewPartnerPostApproveSendsCorrectBody()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DETAIL_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    PartnerPostReviewRequest req;
    req.decision = QStringLiteral("APPROVE");

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    PartnerPostReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.reviewPartnerPost(QStringLiteral("p1"), req, [&](const PartnerPostReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/admin/partner-posts/p1/review"));
    const QJsonObject body = extractBodyJson(captured);
    QCOMPARE(body.value("decision").toString(), QString("APPROVE"));
    QVERIFY2(!body.contains("reason") || body.value("reason").isNull(), "Approve should not include reason");
}

void AdminReviewApiServiceTest::reviewPartnerPostRejectSendsReasonInBody()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DETAIL_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    PartnerPostReviewRequest req;
    req.decision = QStringLiteral("REJECT");
    req.reason = QStringLiteral("needs improvement");

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    PartnerPostReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.reviewPartnerPost(QStringLiteral("p1"), req, [&](const PartnerPostReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    const QJsonObject body = extractBodyJson(captured);
    QCOMPARE(body.value("decision").toString(), QString("REJECT"));
    QCOMPARE(body.value("reason").toString(), QString("needs improvement"));
}

void AdminReviewApiServiceTest::listPendingIdentityVerificationsUsesCorrectPathAndQuery()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", PENDING_IV_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    PendingIdentityVerificationListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listPendingIdentityVerifications(0, 20, [&](const PendingIdentityVerificationListResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    const QByteArray path = extractPath(captured);
    QVERIFY2(path.startsWith("/api/admin/identity-verifications?"), "Path must start with /api/admin/identity-verifications?");
    QVERIFY2(path.contains("status=PENDING_REVIEW"), "Must contain status=PENDING_REVIEW");
    QCOMPARE(result.items.size(), 1);
    QCOMPARE(result.items[0].submissionId, 1);
    QCOMPARE(result.items[0].materialContentType, QString("application/pdf"));
    QCOMPARE(result.items[0].materialSizeBytes, 1024);
}

void AdminReviewApiServiceTest::reviewIdentityVerificationApprovedSendsCorrectBody()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"reviewStatus":"APPROVED","authenticationStatus":"VERIFIED","reviewedAt":"2026-05-24T00:00:00Z","rejectReason":null})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    IdentityVerificationReviewRequest req;
    req.decision = QStringLiteral("APPROVED");

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    IdentityVerificationReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.reviewIdentityVerification(1, req, [&](const IdentityVerificationReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/admin/identity-verifications/1/reviews"));
    const QJsonObject body = extractBodyJson(captured);
    QCOMPARE(body.value("decision").toString(), QString("APPROVED"));
    QCOMPARE(result.reviewStatus, QString("APPROVED"));
    QCOMPARE(result.authenticationStatus, QString("VERIFIED"));
}

void AdminReviewApiServiceTest::reviewIdentityVerificationRejectedSendsRejectReason()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"reviewStatus":"REJECTED","authenticationStatus":"PENDING","reviewedAt":"2026-05-24T00:00:00Z","rejectReason":"unclear photo"})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("admin-token"));
    AdminReviewApiService service(client, tokenStore);

    IdentityVerificationReviewRequest req;
    req.decision = QStringLiteral("REJECTED");
    req.rejectReason = QStringLiteral("unclear photo");

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    IdentityVerificationReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.reviewIdentityVerification(1, req, [&](const IdentityVerificationReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    const QJsonObject body = extractBodyJson(captured);
    QCOMPARE(body.value("decision").toString(), QString("REJECTED"));
    QCOMPARE(body.value("rejectReason").toString(), QString("unclear photo"));
    QCOMPARE(result.reviewStatus, QString("REJECTED"));
    QCOMPARE(result.rejectReason, QString("unclear photo"));
}

void AdminReviewApiServiceTest::errorResponseConvertsToServiceResult()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 403 Forbidden",
        R"({"code":"FORBIDDEN","message":"Admin access required","details":{},"traceId":"t1"})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("non-admin-token"));
    AdminReviewApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    PartnerPostReviewQueueResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listPartnerPostReviewQueue(0, 20, [&](const PartnerPostReviewQueueResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(!result.success);
    QCOMPARE(result.errorCode, QString("FORBIDDEN"));
    QCOMPARE(result.errorMessage, QString("Admin access required"));
}

QTEST_MAIN(AdminReviewApiServiceTest)
#include "AdminReviewApiServiceTest.moc"
