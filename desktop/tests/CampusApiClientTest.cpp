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

private:
    static QUrl serveSingleResponse(const QByteArray &statusLine, const QByteArray &body);
    static ApiClientResponse requestOnce(const QUrl &baseUrl, const QString &path);
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

ApiClientResponse CampusApiClientTest::requestOnce(const QUrl &baseUrl, const QString &path)
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
        response.error.message = QStringLiteral("API client request did not finish before the test timeout");
    }
    return response;
}

void CampusApiClientTest::getJsonParsesSuccessPayload()
{
    const QUrl baseUrl = serveSingleResponse(
        "HTTP/1.1 200 OK",
        R"({"status":"UP","service":"campus-buddy-backend"})");

    QVERIFY(baseUrl.isValid());
    const ApiClientResponse response = requestOnce(baseUrl, "/health");

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
    const ApiClientResponse response = requestOnce(baseUrl, "/missing");

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
        QStringLiteral(CAMPUS_BUDDY_DESKTOP_SOURCE_DIR "/src/main.cpp")
    };

    for (const QString &path : widgetLayerFiles) {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
        const QString content = QString::fromUtf8(file.readAll());
        QVERIFY2(!content.contains("QNetworkAccessManager"),
                 qPrintable(path + " must not directly use QNetworkAccessManager"));
    }
}

QTEST_MAIN(CampusApiClientTest)

#include "CampusApiClientTest.moc"
