#include <QCoreApplication>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QTimer>

#include "api/CampusApiClient.h"
#include "domain/ApiClientConfig.h"

static ApiClientResponse blockingGet(CampusApiClient &client, const QString &path, const QString &token = QString())
{
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    if (token.isEmpty()) {
        client.getJson(path, [&](const ApiClientResponse &result) {
            response = result;
            completed = true;
            loop.quit();
        });
    } else {
        client.getJson(path, token, [&](const ApiClientResponse &result) {
            response = result;
            completed = true;
            loop.quit();
        });
    }

    timeout.start(10000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

static ApiClientResponse blockingPost(CampusApiClient &client, const QString &path, const QJsonObject &body)
{
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

    timeout.start(10000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QString baseUrl = qEnvironmentVariable("CAMPUS_BUDDY_API_BASE_URL",
                                                  QStringLiteral("http://114.116.203.78/api"));
    const ApiClientConfig config(baseUrl, 10000, 1000, true);
    CampusApiClient client(config);

    int failures = 0;

    QTextStream out(stdout);

    out << "=== Qt Server Integration Smoke ===" << Qt::endl;
    out << "Base URL: " << baseUrl << Qt::endl;

    out << Qt::endl << "--- 1. GET /health ---" << Qt::endl;
    ApiClientResponse health = blockingGet(client, "/health");
    if (health.ok && health.json.value("status").toString() == "UP") {
        out << "PASS: status=UP" << Qt::endl;
    } else {
        out << "FAIL: ok=" << health.ok << " status=" << health.json.value("status").toString() << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 2. POST /auth/login ---" << Qt::endl;
    QJsonObject loginBody;
    loginBody["campusEmail"] = "smoketest@campus.edu.cn";
    loginBody["password"] = "SmokeTest123!";
    ApiClientResponse login = blockingPost(client, "/auth/login", loginBody);
    QString accessToken;
    if (login.ok) {
        accessToken = login.json.value("accessToken").toString();
        out << "PASS: token length=" << accessToken.length() << Qt::endl;
    } else {
        out << "FAIL: ok=" << login.ok << " httpStatus=" << login.error.httpStatus << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 3. GET /me/credit-summary (auth) ---" << Qt::endl;
    if (!accessToken.isEmpty()) {
        ApiClientResponse credit = blockingGet(client, "/me/credit-summary", accessToken);
        if (credit.ok && credit.json.contains("averageRating")) {
            out << "PASS: averageRating=" << credit.json.value("averageRating").toDouble()
                << " ratingSampleCount=" << credit.json.value("ratingSampleCount").toInt() << Qt::endl;
        } else {
            out << "FAIL: ok=" << credit.ok << " httpStatus=" << credit.error.httpStatus << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no access token" << Qt::endl;
        failures++;
    }

    out << Qt::endl;
    if (failures == 0) {
        out << "=== ALL INTEGRATION SMOKE TESTS PASSED ===" << Qt::endl;
    } else {
        out << "=== " << failures << " FAILURE(S) ===" << Qt::endl;
    }

    return failures;
}
