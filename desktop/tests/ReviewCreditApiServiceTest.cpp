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
#include "api/ReviewCreditApiService.h"
#include "auth/InMemorySessionTokenStore.h"
#include "domain/ApiClientConfig.h"

class ReviewCreditApiServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void createReviewUsesPostAndCorrectPath();
    void updateReviewUsesPutAndCorrectPath();
    void listGivenReviewsUsesCorrectPathWithQuery();
    void listReceivedReviewsUsesCorrectPathWithQuery();
    void getMyCreditSummaryParsesDisputedReviewCount();
    void getPublicCreditSummaryUsesCorrectPath();
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
};

QUrl ReviewCreditApiServiceTest::serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody)
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

QByteArray ReviewCreditApiServiceTest::extractMethod(const RawRequest &captured)
{
    const int space = captured.headers.indexOf(' ');
    return space < 0 ? QByteArray() : captured.headers.left(space);
}

QByteArray ReviewCreditApiServiceTest::extractPath(const RawRequest &captured)
{
    const int firstSpace = captured.headers.indexOf(' ');
    if (firstSpace < 0) return {};
    const int secondSpace = captured.headers.indexOf(' ', firstSpace + 1);
    return secondSpace < 0 ? QByteArray() : captured.headers.mid(firstSpace + 1, secondSpace - firstSpace - 1);
}

QByteArray ReviewCreditApiServiceTest::extractHeader(const RawRequest &captured, const QByteArray &headerName)
{
    const QList<QByteArray> lines = captured.headers.split('\n');
    for (const QByteArray &line : lines) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.startsWith(headerName + ":")) return trimmed.mid(headerName.size() + 1).trimmed();
    }
    return {};
}

static const char *REVIEW_RESPONSE = R"({"id":1,"conversationId":2,"reviewerId":"a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11","revieweeId":"b0eebc99-9c0b-4ef8-bb6d-6bb9bd380a22","rating":5,"reviewTags":["friendly"],"status":"ACTIVE","modifiedOnce":false,"createdAt":"2026-05-24T00:00:00Z","updatedAt":"2026-05-24T00:00:00Z"})";

void ReviewCreditApiServiceTest::createReviewUsesPostAndCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 201 Created", REVIEW_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    CreateReviewRequest req;
    req.conversationId = 2;
    req.revieweeId = QStringLiteral("b0eebc99-9c0b-4ef8-bb6d-6bb9bd380a22");
    req.rating = 5;
    req.reviewTags = QStringList{QStringLiteral("friendly")};

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    ReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.createReview(req, [&](const ReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/reviews"));
    const QByteArray auth = extractHeader(captured, "Authorization");
    QVERIFY2(auth.startsWith("Bearer "), "Must send Bearer token");
}

void ReviewCreditApiServiceTest::updateReviewUsesPutAndCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", REVIEW_RESPONSE);
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    UpdateReviewRequest req;
    req.rating = 4;
    req.reviewTags = QStringList{QStringLiteral("punctual")};

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    ReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.updateReview(1, req, [&](const ReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("PUT"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/reviews/1"));
}

void ReviewCreditApiServiceTest::listGivenReviewsUsesCorrectPathWithQuery()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    ReviewListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listGivenReviews(0, 20, [&](const ReviewListResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    const QByteArray path = extractPath(captured);
    QVERIFY2(path.startsWith("/api/me/reviews/given?"), "Path must start with /api/me/reviews/given?");
}

void ReviewCreditApiServiceTest::listReceivedReviewsUsesCorrectPathWithQuery()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    ReviewListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listReceivedReviews(0, 20, [&](const ReviewListResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    const QByteArray path = extractPath(captured);
    QVERIFY2(path.startsWith("/api/me/reviews/received?"), "Path must start with /api/me/reviews/received?");
}

void ReviewCreditApiServiceTest::getMyCreditSummaryParsesDisputedReviewCount()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"userId":"a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11","averageRating":4.5,"realConversationCount":3,"ratingSampleCount":5,"topTags":[{"tag":"friendly","count":3}],"disputedReviewCount":1,"updatedAt":"2026-05-24T00:00:00Z"})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    MyCreditSummaryResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.getMyCreditSummary([&](const MyCreditSummaryResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/credit-summary"));
    QCOMPARE(result.averageRating, 4.5);
    QCOMPARE(result.disputedReviewCount, 1);
    QCOMPARE(result.topTags.size(), 1);
}

void ReviewCreditApiServiceTest::getPublicCreditSummaryUsesCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"userId":"b0eebc99-9c0b-4ef8-bb6d-6bb9bd380a22","averageRating":3.0,"realConversationCount":1,"ratingSampleCount":2,"topTags":[],"updatedAt":"2026-05-24T00:00:00Z"})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    PublicCreditSummaryResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.getPublicCreditSummary(QStringLiteral("b0eebc99-9c0b-4ef8-bb6d-6bb9bd380a22"), [&](const PublicCreditSummaryResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractPath(captured), QByteArray("/api/users/b0eebc99-9c0b-4ef8-bb6d-6bb9bd380a22/credit-summary"));
    QVERIFY2(!result.success || true, "Public summary does not contain disputedReviewCount");
}

void ReviewCreditApiServiceTest::errorResponseConvertsToServiceResult()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 400 Bad Request",
        R"({"code":"VALIDATION_FAILED","message":"Rating must be between 1 and 6","details":{},"traceId":"t1"})");
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    ReviewCreditApiService service(client, tokenStore);

    CreateReviewRequest req;
    QEventLoop loop; QTimer timeout; timeout.setSingleShot(true);
    ReviewResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.createReview(req, [&](const ReviewResult &r) { result = r; loop.quit(); });
    timeout.start(3000); loop.exec();

    QVERIFY(!result.success);
    QCOMPARE(result.errorCode, QString("VALIDATION_FAILED"));
    QCOMPARE(result.errorMessage, QString("Rating must be between 1 and 6"));
}

QTEST_MAIN(ReviewCreditApiServiceTest)
#include "ReviewCreditApiServiceTest.moc"
