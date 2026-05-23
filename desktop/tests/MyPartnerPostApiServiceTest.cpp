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
#include "api/MyPartnerPostApiService.h"
#include "auth/InMemorySessionTokenStore.h"
#include "domain/ApiClientConfig.h"

class MyPartnerPostApiServiceTest : public QObject
{
    Q_OBJECT

private slots:
    void createDraftUsesPostAndCorrectPath();
    void createDraftSendsBearerToken();
    void createDraftSendsDraftFieldsInBody();
    void updateDraftUsesPutAndCorrectPath();
    void listMyPostsUsesCorrectPathWithQuery();
    void listMyPostsWithStatusFilter();
    void listMyPostsUsesBearerToken();
    void getMyPostDetailParsesAllowedActionsAndScenePayload();
    void submitReviewUsesCorrectPath();
    void withdrawReviewUsesCorrectPath();
    void unpublishUsesCorrectPath();
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

QUrl MyPartnerPostApiServiceTest::serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody)
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
            if (headerEnd < 0) {
                return;
            }

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

QByteArray MyPartnerPostApiServiceTest::extractMethod(const RawRequest &captured)
{
    const int space = captured.headers.indexOf(' ');
    if (space < 0) return {};
    return captured.headers.left(space);
}

QByteArray MyPartnerPostApiServiceTest::extractPath(const RawRequest &captured)
{
    const int firstSpace = captured.headers.indexOf(' ');
    if (firstSpace < 0) return {};
    const int secondSpace = captured.headers.indexOf(' ', firstSpace + 1);
    if (secondSpace < 0) return {};
    return captured.headers.mid(firstSpace + 1, secondSpace - firstSpace - 1);
}

QByteArray MyPartnerPostApiServiceTest::extractHeader(const RawRequest &captured, const QByteArray &headerName)
{
    const QList<QByteArray> lines = captured.headers.split('\n');
    for (const QByteArray &line : lines) {
        const QByteArray trimmed = line.trimmed();
        if (trimmed.startsWith(headerName + ":")) {
            return trimmed.mid(headerName.size() + 1).trimmed();
        }
    }
    return {};
}

static const char *DRAFT_RESPONSE_JSON = R"({"postId":"abc-123","publisherId":"u1","sceneType":"STUDY","status":"DRAFT","title":"Test","description":"desc","timeMode":"TEXT_PREFERENCE","timeText":"weekend","startAt":null,"endAt":null,"locationText":"library","participantCount":2,"targetRequirement":"any","contactPreference":"in-app","tags":["math"],"attachmentIds":[],"scenePayload":{"studyGoal":"pass exam"},"rejectReason":null,"publishedAt":null,"createdAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","allowedActions":["UPDATE_DRAFT","SUBMIT_REVIEW","DELETE"]})";

void MyPartnerPostApiServiceTest::createDraftUsesPostAndCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    req.sceneType = QStringLiteral("STUDY");
    req.title = QStringLiteral("Test");

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.createDraft(req, [&](const MyPostResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/partner-posts"));
}

void MyPartnerPostApiServiceTest::createDraftSendsBearerToken()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("my-bearer-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.createDraft(req, [&](const MyPostResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    const QByteArray authHeader = extractHeader(captured, "Authorization");
    QVERIFY2(authHeader.startsWith("Bearer "), "Must send Authorization Bearer header");
    QVERIFY2(authHeader.contains("my-bearer-token"), "Bearer token must match");
}

void MyPartnerPostApiServiceTest::createDraftSendsDraftFieldsInBody()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    req.sceneType = QStringLiteral("STUDY");
    req.title = QStringLiteral("Study Buddy");
    req.description = QStringLiteral("Looking for a study partner");
    req.timeMode = QStringLiteral("TEXT_PREFERENCE");
    req.timeText = QStringLiteral("weekends");
    req.locationText = QStringLiteral("Library");
    req.participantCount = 3;
    req.targetRequirement = QStringLiteral("GPA > 3.0");
    req.contactPreference = QStringLiteral("in-app chat");
    req.tags = QStringList{QStringLiteral("math"), QStringLiteral("physics")};
    req.attachmentIds = QStringList{QStringLiteral("att-1")};
    req.scenePayload.insert(QStringLiteral("studyGoal"), QStringLiteral("pass exam"));

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.createDraft(req, [&](const MyPostResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    const QJsonObject sent = doc.object();

    QCOMPARE(sent.value("sceneType").toString(), QString("STUDY"));
    QCOMPARE(sent.value("title").toString(), QString("Study Buddy"));
    QCOMPARE(sent.value("description").toString(), QString("Looking for a study partner"));
    QCOMPARE(sent.value("timeMode").toString(), QString("TEXT_PREFERENCE"));
    QCOMPARE(sent.value("timeText").toString(), QString("weekends"));
    QCOMPARE(sent.value("locationText").toString(), QString("Library"));
    QCOMPARE(sent.value("participantCount").toInt(), 3);
    QCOMPARE(sent.value("targetRequirement").toString(), QString("GPA > 3.0"));
    QCOMPARE(sent.value("tags").toArray().size(), 2);
    QCOMPARE(sent.value("attachmentIds").toArray().size(), 1);
    QVERIFY(sent.contains("scenePayload"));
    QCOMPARE(sent.value("scenePayload").toObject().value("studyGoal").toString(), QString("pass exam"));
}

void MyPartnerPostApiServiceTest::updateDraftUsesPutAndCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    req.sceneType = QStringLiteral("STUDY");
    req.title = QStringLiteral("Updated Title");
    req.timeMode = QStringLiteral("TEXT_PREFERENCE");
    req.timeText = QStringLiteral("weekends");
    req.locationText = QStringLiteral("Library");
    req.participantCount = 2;
    req.targetRequirement = QStringLiteral("any");
    req.contactPreference = QStringLiteral("in-app");
    req.scenePayload.insert(QStringLiteral("studyGoal"), QStringLiteral("pass"));

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.updateDraft(QStringLiteral("abc-123"), req, [&](const MyPostResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("PUT"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/partner-posts/abc-123"));

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value("title").toString(), QString("Updated Title"));
}

void MyPartnerPostApiServiceTest::listMyPostsUsesCorrectPathWithQuery()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listMyPosts(0, 20, [&](const MyPostListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    const QByteArray path = extractPath(captured);
    QVERIFY2(path.startsWith("/api/me/partner-posts?"), "Path must start with /api/me/partner-posts?");
    QVERIFY2(path.contains("page=0"), "Query must contain page=0");
    QVERIFY2(path.contains("size=20"), "Query must contain size=20");
}

void MyPartnerPostApiServiceTest::listMyPostsWithStatusFilter()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":10,"totalElements":0,"totalPages":0})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listMyPosts(QStringLiteral("DRAFT"), 0, 10, [&](const MyPostListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    const QByteArray path = extractPath(captured);
    QVERIFY2(path.contains("status=DRAFT"), "Query must contain status=DRAFT");
    QVERIFY2(path.contains("page=0"), "Query must contain page=0");
    QVERIFY2(path.contains("size=10"), "Query must contain size=10");
}

void MyPartnerPostApiServiceTest::listMyPostsUsesBearerToken()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"items":[],"page":0,"size":20,"totalElements":0,"totalPages":0})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("my-bearer-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostListResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.listMyPosts(0, 20, [&](const MyPostListResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    const QByteArray authHeader = extractHeader(captured, "Authorization");
    QVERIFY2(authHeader.startsWith("Bearer "), "Must send Authorization Bearer header for list my posts");
}

void MyPartnerPostApiServiceTest::getMyPostDetailParsesAllowedActionsAndScenePayload()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.getMyPostDetail(QStringLiteral("abc-123"), [&](const MyPostResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("GET"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/partner-posts/abc-123"));
    QCOMPARE(result.post.allowedActions.size(), 3);
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("DELETE")));
    QVERIFY(result.post.scenePayload.contains(QStringLiteral("studyGoal")));
    QCOMPARE(result.post.scenePayload.value(QStringLiteral("studyGoal")).toString(), QString("pass exam"));
}

void MyPartnerPostApiServiceTest::submitReviewUsesCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"postId":"abc-123","publisherId":"u1","sceneType":"STUDY","status":"PENDING_REVIEW","title":"Test","description":"desc","timeMode":"TEXT_PREFERENCE","timeText":"weekend","startAt":null,"endAt":null,"locationText":"library","participantCount":2,"targetRequirement":"any","contactPreference":"in-app","tags":[],"attachmentIds":[],"scenePayload":{},"rejectReason":null,"publishedAt":null,"createdAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","allowedActions":["WITHDRAW_REVIEW"]})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PostActionResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.submitReview(QStringLiteral("abc-123"), [&](const PostActionResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/partner-posts/abc-123/submit-review"));
    QCOMPARE(result.post.status, QString("PENDING_REVIEW"));
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("WITHDRAW_REVIEW")));

    QVERIFY2(!captured.body.contains("token"), "Request body must not contain token");
    QVERIFY2(!captured.body.contains("password"), "Request body must not contain password");
}

void MyPartnerPostApiServiceTest::withdrawReviewUsesCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PostActionResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.withdrawReview(QStringLiteral("abc-123"), [&](const PostActionResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/partner-posts/abc-123/withdraw-review"));
    QCOMPARE(result.post.status, QString("DRAFT"));
}

void MyPartnerPostApiServiceTest::unpublishUsesCorrectPath()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured, "HTTP/1.1 200 OK",
        R"({"postId":"abc-123","publisherId":"u1","sceneType":"STUDY","status":"DRAFT","title":"Test","description":"desc","timeMode":"TEXT_PREFERENCE","timeText":"weekend","startAt":null,"endAt":null,"locationText":"library","participantCount":2,"targetRequirement":"any","contactPreference":"in-app","tags":[],"attachmentIds":[],"scenePayload":{},"rejectReason":null,"publishedAt":null,"createdAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","allowedActions":["UPDATE_DRAFT","SUBMIT_REVIEW","DELETE"]})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    PostActionResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.unpublish(QStringLiteral("abc-123"), [&](const PostActionResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(result.success);
    QCOMPARE(extractMethod(captured), QByteArray("POST"));
    QCOMPARE(extractPath(captured), QByteArray("/api/me/partner-posts/abc-123/unpublish"));
}

void MyPartnerPostApiServiceTest::errorResponseConvertsToServiceResult()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 400 Bad Request",
        R"({"code":"VALIDATION_FAILED","message":"Validation failed","details":{"title":"must not be blank"},"traceId":"trace-1"})");

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    MyPostResult result;
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    service.createDraft(req, [&](const MyPostResult &r) {
        result = r;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    QVERIFY(!result.success);
    QCOMPARE(result.errorCode, QString("VALIDATION_FAILED"));
    QCOMPARE(result.errorMessage, QString("Validation failed"));
}

QTEST_MAIN(MyPartnerPostApiServiceTest)

#include "MyPartnerPostApiServiceTest.moc"
