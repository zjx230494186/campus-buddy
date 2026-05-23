#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QHostAddress>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "api/CampusApiClient.h"

class CampusApiClientTest : public QObject
{
    Q_OBJECT

private slots:
    void getJsonParsesSuccessPayload();
    void errorResponseConvertsToClientError();
    void widgetLayerDoesNotDirectlyUseNetworkAccessManager();
    void postJsonSendsBodyAndParsesResponse();
    void getJsonWithAuthSendsAuthorizationHeader();
    void postJsonWithAuthSendsAuthorizationHeader();
    void sendVerificationCodeRequestBodyContainsPurpose();
    void verifyCampusEmailRequestBodyContainsPurpose();
    void registerRequestBodyContainsVerificationTicketAndDisplayName();
    void submitIdentityVerificationRequestBodyContainsAllFields();
    void identityVerificationStatusResponseParsesAllFields();

private:
    struct RawRequest {
        QByteArray headers;
        QByteArray body;
    };

    static QUrl serveSingleResponse(const QByteArray &statusLine, const QByteArray &body);
    static QUrl serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody);
    static ApiClientResponse requestGet(const QUrl &baseUrl, const QString &path);
    static ApiClientResponse requestGetWithAuth(const QUrl &baseUrl, const QString &path, const QString &token);
    static ApiClientResponse requestPost(const QUrl &baseUrl, const QString &path, const QJsonObject &body);
    static ApiClientResponse requestPostWithAuth(const QUrl &baseUrl, const QString &path, const QJsonObject &body, const QString &token);
    static RawRequest capturePost(const QUrl &baseUrl, const QString &path, const QJsonObject &body, const QByteArray &responseBody);
};

QUrl CampusApiClientTest::serveSingleResponse(const QByteArray &statusLine, const QByteArray &body)
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
            if (!request.contains("\r\n\r\n")) {
                return;
            }

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

QUrl CampusApiClientTest::serveAndCaptureRequest(RawRequest &captured, const QByteArray &responseStatus, const QByteArray &responseBody)
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

ApiClientResponse CampusApiClientTest::requestGet(const QUrl &baseUrl, const QString &path)
{
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.getJson(path, [&](const ApiClientResponse &result) {
        response = result;
        completed = true;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

ApiClientResponse CampusApiClientTest::requestGetWithAuth(const QUrl &baseUrl, const QString &path, const QString &token)
{
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.getJson(path, token, [&](const ApiClientResponse &result) {
        response = result;
        completed = true;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

ApiClientResponse CampusApiClientTest::requestPost(const QUrl &baseUrl, const QString &path, const QJsonObject &body)
{
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.postJson(path, body, [&](const ApiClientResponse &result) {
        response = result;
        completed = true;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

ApiClientResponse CampusApiClientTest::requestPostWithAuth(const QUrl &baseUrl, const QString &path, const QJsonObject &body, const QString &token)
{
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.postJson(path, body, token, [&](const ApiClientResponse &result) {
        response = result;
        completed = true;
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

CampusApiClientTest::RawRequest CampusApiClientTest::capturePost(const QUrl &baseUrl, const QString &path, const QJsonObject &body, const QByteArray &responseBody)
{
    CampusApiClient client(ApiClientConfig(baseUrl.toString(), 1000, 1000, true));
    RawRequest captured;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.postJson(path, body, [&](const ApiClientResponse &) {
        loop.quit();
    });

    timeout.start(3000);
    loop.exec();

    Q_UNUSED(captured)
    return captured;
}

void CampusApiClientTest::getJsonParsesSuccessPayload()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"status":"UP","service":"campus-buddy-backend"})");

    QVERIFY(baseUrl.isValid());
    const ApiClientResponse response = requestGet(baseUrl, "/health");

    QVERIFY(response.ok);
    QVERIFY(!response.error.hasError());
    QCOMPARE(response.json.value("status").toString(), QString("UP"));
    QCOMPARE(response.json.value("service").toString(), QString("campus-buddy-backend"));
}

void CampusApiClientTest::errorResponseConvertsToClientError()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 404 Not Found",
        R"({"code":"NOT_FOUND","message":"Resource not found","details":{"path":"/missing"},"traceId":"trace-api-client-test"})");

    QVERIFY(baseUrl.isValid());
    const ApiClientResponse response = requestGet(baseUrl, "/missing");

    QVERIFY(!response.ok);
    QVERIFY(response.error.hasError());
    QCOMPARE(response.error.type, ApiClientError::HttpError);
    QCOMPARE(response.error.httpStatus, 404);
    QCOMPARE(response.error.code, QString("NOT_FOUND"));
    QCOMPARE(response.error.message, QString("Resource not found"));
    QCOMPARE(response.error.traceId, QString("trace-api-client-test"));
    QCOMPARE(response.error.details.value("path").toString(), QString("/missing"));
}

void CampusApiClientTest::widgetLayerDoesNotDirectlyUseNetworkAccessManager()
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
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/ui/ConversationsWidget.cpp")
    };

    for (const QString &path : widgetLayerFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(!content.contains("QNetworkAccessManager"),
                 qPrintable(path + " must not directly use QNetworkAccessManager"));
    }
}

void CampusApiClientTest::postJsonSendsBodyAndParsesResponse()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"accessToken":"jwt-123","tokenType":"Bearer"})");

    QVERIFY(baseUrl.isValid());

    QJsonObject body;
    body["campusEmail"] = "test@edu.cn";
    body["password"] = "secret";

    const ApiClientResponse response = requestPost(baseUrl, "/auth/login", body);

    QVERIFY(response.ok);
    QCOMPARE(response.json.value("accessToken").toString(), QString("jwt-123"));
}

void CampusApiClientTest::getJsonWithAuthSendsAuthorizationHeader()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"authenticationStatus":"UNVERIFIED"})");

    QVERIFY(baseUrl.isValid());

    const ApiClientResponse response = requestGetWithAuth(baseUrl, "/auth/identity-verifications/me", QStringLiteral("my-jwt-access-token"));

    QVERIFY(response.ok);

    const QString headerStr = QString::fromUtf8(captured.headers);
    QVERIFY2(headerStr.contains("Authorization: Bearer my-jwt-access-token"),
             "GET request with auth must include Authorization Bearer header");
}

void CampusApiClientTest::postJsonWithAuthSendsAuthorizationHeader()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"authenticationStatus":"PENDING_REVIEW"})");

    QVERIFY(baseUrl.isValid());

    QJsonObject body;
    body["realName"] = "Test";
    body["studentNumber"] = "2024001";

    const ApiClientResponse response = requestPostWithAuth(baseUrl, "/auth/identity-verifications", body, QStringLiteral("my-jwt-access-token"));

    QVERIFY(response.ok);

    const QString headerStr = QString::fromUtf8(captured.headers);
    QVERIFY2(headerStr.contains("Authorization: Bearer my-jwt-access-token"),
             "POST request with auth must include Authorization Bearer header");
}

void CampusApiClientTest::sendVerificationCodeRequestBodyContainsPurpose()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"campusEmail":"test@edu.cn","verificationStatus":"CODE_SENT","expiresInSeconds":600,"resendAfterSeconds":60})");

    QVERIFY(baseUrl.isValid());

    QJsonObject body;
    body["campusEmail"] = "test@edu.cn";
    body["purpose"] = "REGISTER_OR_LOGIN";

    const ApiClientResponse response = requestPost(baseUrl, "/auth/campus-email/verification-codes", body);
    QVERIFY(response.ok);

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value("purpose").toString(), QString("REGISTER_OR_LOGIN"));
    QCOMPARE(doc.object().value("campusEmail").toString(), QString("test@edu.cn"));
}

void CampusApiClientTest::verifyCampusEmailRequestBodyContainsPurpose()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"campusEmail":"test@edu.cn","verificationStatus":"VERIFIED","verifiedAt":"2026-05-19T00:00:00Z","verificationTicket":"ticket-abc"})");

    QVERIFY(baseUrl.isValid());

    QJsonObject body;
    body["campusEmail"] = "test@edu.cn";
    body["code"] = "123456";
    body["purpose"] = "REGISTER_OR_LOGIN";

    const ApiClientResponse response = requestPost(baseUrl, "/auth/campus-email/verifications", body);
    QVERIFY(response.ok);

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    QCOMPARE(doc.object().value("purpose").toString(), QString("REGISTER_OR_LOGIN"));
    QCOMPARE(doc.object().value("campusEmail").toString(), QString("test@edu.cn"));
    QCOMPARE(doc.object().value("code").toString(), QString("123456"));
}

void CampusApiClientTest::registerRequestBodyContainsVerificationTicketAndDisplayName()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"userId":"uid-1","campusEmail":"test@edu.cn","displayName":"MyName","authenticationStatus":"UNVERIFIED","campusEmailVerificationStatus":"VERIFIED","createdAt":"2026-05-19T00:00:00Z"})");

    QVERIFY(baseUrl.isValid());

    QJsonObject body;
    body["campusEmail"] = "test@edu.cn";
    body["verificationTicket"] = "ticket-abc";
    body["password"] = "secret123";
    body["displayName"] = "MyName";

    const ApiClientResponse response = requestPost(baseUrl, "/auth/register", body);
    QVERIFY(response.ok);

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    const QJsonObject sent = doc.object();

    QCOMPARE(sent.value("campusEmail").toString(), QString("test@edu.cn"));
    QCOMPARE(sent.value("verificationTicket").toString(), QString("ticket-abc"));
    QCOMPARE(sent.value("password").toString(), QString("secret123"));
    QCOMPARE(sent.value("displayName").toString(), QString("MyName"));

    QVERIFY2(!sent.contains("realName"),
             "register request body must NOT contain realName");
    QVERIFY2(!sent.contains("studentNumber"),
             "register request body must NOT contain studentNumber");
    QVERIFY2(!sent.contains("verificationCode"),
             "register request body must NOT contain verificationCode");
}

void CampusApiClientTest::submitIdentityVerificationRequestBodyContainsAllFields()
{
    RawRequest captured;
    const QUrl baseUrl = serveAndCaptureRequest(captured,
        "HTTP/1.1 200 OK",
        R"({"authenticationStatus":"PENDING_REVIEW","submittedAt":"2026-05-19T00:00:00Z","realName":"Zhang","studentNumber":"2024001","college":"CS","major":"SE","grade":"2024"})");

    QVERIFY(baseUrl.isValid());

    QJsonObject body;
    body["realName"] = "Zhang";
    body["studentNumber"] = "2024001";
    body["college"] = "CS";
    body["major"] = "SE";
    body["grade"] = "2024";
    body["materialAttachmentId"] = "att-uuid-123";

    const ApiClientResponse response = requestPostWithAuth(baseUrl, "/auth/identity-verifications", body, QStringLiteral("my-jwt-token"));
    QVERIFY(response.ok);

    const QJsonDocument doc = QJsonDocument::fromJson(captured.body, nullptr);
    QVERIFY(doc.isObject());
    const QJsonObject sent = doc.object();

    QCOMPARE(sent.value("realName").toString(), QString("Zhang"));
    QCOMPARE(sent.value("studentNumber").toString(), QString("2024001"));
    QCOMPARE(sent.value("college").toString(), QString("CS"));
    QCOMPARE(sent.value("major").toString(), QString("SE"));
    QCOMPARE(sent.value("grade").toString(), QString("2024"));
    QCOMPARE(sent.value("materialAttachmentId").toString(), QString("att-uuid-123"));
}

void CampusApiClientTest::identityVerificationStatusResponseParsesAllFields()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"authenticationStatus":"REJECTED","reviewStatus":"REJECTED","submittedAt":"2026-05-19T00:00:00Z","reviewedAt":"2026-05-19T01:00:00Z","rejectReason":"Material unclear","realName":"Zhang","studentNumber":"2024001","college":"CS","major":"SE","grade":"2024","allowedActions":["SUBMIT"]})");

    QVERIFY(baseUrl.isValid());

    const ApiClientResponse response = requestGetWithAuth(baseUrl, "/auth/identity-verifications/me", QStringLiteral("my-jwt-token"));
    QVERIFY(response.ok);

    QCOMPARE(response.json.value("authenticationStatus").toString(), QString("REJECTED"));
    QCOMPARE(response.json.value("reviewStatus").toString(), QString("REJECTED"));
    QCOMPARE(response.json.value("rejectReason").toString(), QString("Material unclear"));
    QCOMPARE(response.json.value("allowedActions").toArray().size(), 1);
    QCOMPARE(response.json.value("allowedActions").toArray()[0].toString(), QString("SUBMIT"));
}

QTEST_MAIN(CampusApiClientTest)

#include "CampusApiClientTest.moc"
