#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QEventLoop>
#include <QHostAddress>
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
    static QUrl serveSingleResponse(const QByteArray &statusLine, const QByteArray &body);
};

QUrl MyPartnerPostApiServiceTest::serveSingleResponse(const QByteArray &statusLine, const QByteArray &body)
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

static const char *DRAFT_RESPONSE_JSON = R"({"postId":"abc-123","publisherId":"u1","sceneType":"STUDY","status":"DRAFT","title":"Test","description":"desc","timeMode":"FIXED","timeText":"weekend","startAt":null,"endAt":null,"locationText":"library","participantCount":2,"targetRequirement":"any","contactPreference":"in-app","tags":["math"],"attachmentIds":[],"scenePayload":{"studyGoal":"pass exam"},"rejectReason":null,"publishedAt":null,"createdAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","allowedActions":["UPDATE_DRAFT","SUBMIT_REVIEW","DELETE"]})";

void MyPartnerPostApiServiceTest::createDraftUsesPostAndCorrectPath()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

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
    QCOMPARE(result.post.postId, QString("abc-123"));
    QCOMPARE(result.post.status, QString("DRAFT"));
}

void MyPartnerPostApiServiceTest::createDraftSendsBearerToken()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

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
}

void MyPartnerPostApiServiceTest::createDraftSendsDraftFieldsInBody()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    req.sceneType = QStringLiteral("STUDY");
    req.title = QStringLiteral("Study Buddy");
    req.description = QStringLiteral("Looking for a study partner");
    req.timeMode = QStringLiteral("FIXED");
    req.timeText = QStringLiteral("weekends");
    req.locationText = QStringLiteral("Library");
    req.participantCount = 3;
    req.targetRequirement = QStringLiteral("GPA > 3.0");
    req.tags = QStringList{QStringLiteral("math"), QStringLiteral("physics")};
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
}

void MyPartnerPostApiServiceTest::updateDraftUsesPutAndCorrectPath()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    InMemorySessionTokenStore tokenStore;
    tokenStore.setAccessToken(QStringLiteral("test-token"));
    MyPartnerPostApiService service(client, tokenStore);

    MyPostDraftRequest req;
    req.title = QStringLiteral("Updated Title");

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
}

void MyPartnerPostApiServiceTest::listMyPostsUsesCorrectPathWithQuery()
{
    const QUrl baseUrl = serveSingleResponse(
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
    QCOMPARE(result.page, 0);
    QCOMPARE(result.size, 20);
}

void MyPartnerPostApiServiceTest::listMyPostsWithStatusFilter()
{
    const QUrl baseUrl = serveSingleResponse(
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
}

void MyPartnerPostApiServiceTest::listMyPostsUsesBearerToken()
{
    const QUrl baseUrl = serveSingleResponse(
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
}

void MyPartnerPostApiServiceTest::getMyPostDetailParsesAllowedActionsAndScenePayload()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

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
    QCOMPARE(result.post.allowedActions.size(), 3);
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("UPDATE_DRAFT")));
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW")));
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("DELETE")));
    QVERIFY(result.post.scenePayload.contains(QStringLiteral("studyGoal")));
    QCOMPARE(result.post.scenePayload.value(QStringLiteral("studyGoal")).toString(), QString("pass exam"));
}

void MyPartnerPostApiServiceTest::submitReviewUsesCorrectPath()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK",
        R"({"postId":"abc-123","publisherId":"u1","sceneType":"STUDY","status":"PENDING_REVIEW","title":"Test","description":"desc","timeMode":"FIXED","timeText":"weekend","startAt":null,"endAt":null,"locationText":"library","participantCount":2,"targetRequirement":"any","contactPreference":"in-app","tags":[],"attachmentIds":[],"scenePayload":{},"rejectReason":null,"publishedAt":null,"createdAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","allowedActions":["WITHDRAW_REVIEW"]})");

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
    QCOMPARE(result.post.status, QString("PENDING_REVIEW"));
    QVERIFY(result.post.allowedActions.contains(QStringLiteral("WITHDRAW_REVIEW")));
}

void MyPartnerPostApiServiceTest::withdrawReviewUsesCorrectPath()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK", DRAFT_RESPONSE_JSON);

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
    QCOMPARE(result.post.status, QString("DRAFT"));
}

void MyPartnerPostApiServiceTest::unpublishUsesCorrectPath()
{
    const QUrl baseUrl = serveSingleResponse("HTTP/1.1 200 OK",
        R"({"postId":"abc-123","publisherId":"u1","sceneType":"STUDY","status":"DRAFT","title":"Test","description":"desc","timeMode":"FIXED","timeText":"weekend","startAt":null,"endAt":null,"locationText":"library","participantCount":2,"targetRequirement":"any","contactPreference":"in-app","tags":[],"attachmentIds":[],"scenePayload":{},"rejectReason":null,"publishedAt":null,"createdAt":"2026-05-23T00:00:00Z","updatedAt":"2026-05-23T00:00:00Z","allowedActions":["UPDATE_DRAFT","SUBMIT_REVIEW","DELETE"]})");

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
}

void MyPartnerPostApiServiceTest::errorResponseConvertsToServiceResult()
{
    const QUrl baseUrl = serveSingleResponse(
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
