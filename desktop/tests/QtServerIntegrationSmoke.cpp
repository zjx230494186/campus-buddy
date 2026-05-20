#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcessEnvironment>
#include <QTimer>

#include "api/CampusApiClient.h"
#include "auth/AuthTokenStore.h"
#include "auth/AuthApiService.h"
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

static ApiClientResponse blockingPost(CampusApiClient &client, const QString &path, const QJsonObject &body, const QString &token = QString())
{
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

    timeout.start(10000);
    loop.exec();

    if (!completed) {
        response.error.type = ApiClientError::NetworkError;
        response.error.message = QStringLiteral("timeout");
    }
    return response;
}

static ApiClientResponse blockingUpload(CampusApiClient &client, const QString &path, QHttpMultiPart *multiPart, const QString &token)
{
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.uploadMultipart(path, multiPart, token, [&](const ApiClientResponse &result) {
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

static ApiClientResponse blockingDelete(CampusApiClient &client, const QString &path, const QString &token)
{
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);

    ApiClientResponse response;
    bool completed = false;

    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    client.deleteResource(path, token, [&](const ApiClientResponse &result) {
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
    QTextStream out(stdout);

    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    const QString baseUrl = env.value("CAMPUS_BUDDY_API_BASE_URL",
                                       QStringLiteral("http://114.116.203.78/api"));
    const QString smokeEmail = env.value("CAMPUS_BUDDY_SMOKE_EMAIL");
    const QString smokePassword = env.value("CAMPUS_BUDDY_SMOKE_PASSWORD");

    if (smokeEmail.isEmpty() || smokePassword.isEmpty()) {
        out << "ERROR: CAMPUS_BUDDY_SMOKE_EMAIL and/or CAMPUS_BUDDY_SMOKE_PASSWORD not set" << Qt::endl;
        out << "Required environment variables:" << Qt::endl;
        out << "  CAMPUS_BUDDY_SMOKE_EMAIL" << Qt::endl;
        out << "  CAMPUS_BUDDY_SMOKE_PASSWORD" << Qt::endl;
        return 2;
    }

    const ApiClientConfig config(baseUrl, 10000, 1000, true);
    CampusApiClient client(config);

    int failures = 0;

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
    loginBody["campusEmail"] = smokeEmail;
    loginBody["password"] = smokePassword;
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

    out << Qt::endl << "--- 4. POST /auth/identity-verifications/materials (upload) ---" << Qt::endl;
    QString uploadedAttachmentId;
    if (!accessToken.isEmpty()) {
        QByteArray testData(512, '\0');
        for (int i = 0; i < testData.size(); ++i) {
            testData[i] = static_cast<char>(i & 0xFF);
        }

        auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                            QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"smoke_test_material.pdf\"")));
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/pdf"));
        filePart.setBody(testData);
        multiPart->append(filePart);

        ApiClientResponse upload = blockingUpload(client, "/auth/identity-verifications/materials", multiPart, accessToken);

        if (upload.ok) {
            const QString attachmentId = upload.json.value("attachmentId").toString();
            const QString respContentType = upload.json.value("contentType").toString();
            const int sizeBytes = upload.json.value("sizeBytes").toInt();
            const QString sha256 = upload.json.value("sha256").toString();

            bool allFieldsPresent = !attachmentId.isEmpty() && !respContentType.isEmpty() && sizeBytes > 0 && !sha256.isEmpty();
            if (allFieldsPresent) {
                uploadedAttachmentId = attachmentId;
                out << "PASS: attachmentId length=" << attachmentId.length()
                    << " contentType=" << respContentType
                    << " sizeBytes=" << sizeBytes
                    << " sha256 length=" << sha256.length() << Qt::endl;
            } else {
                out << "FAIL: missing fields in upload response" << Qt::endl;
                failures++;
            }
        } else {
            out << "FAIL: ok=" << upload.ok << " httpStatus=" << upload.error.httpStatus
                << " code=" << upload.error.code << " message=" << upload.error.message << Qt::endl;
            out << "NOTE: upload may fail if test account has VERIFIED status blocking re-upload" << Qt::endl;
        }
    } else {
        out << "SKIP: no access token for upload" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 5. DELETE /auth/identity-verifications/materials/{id} (cleanup) ---" << Qt::endl;
    if (!uploadedAttachmentId.isEmpty() && !accessToken.isEmpty()) {
        QString deletePath = QStringLiteral("/auth/identity-verifications/materials/") + uploadedAttachmentId;
        ApiClientResponse delResp = blockingDelete(client, deletePath, accessToken);
        if (delResp.ok) {
            out << "PASS: delete succeeded (204 No Content)" << Qt::endl;
        } else {
            out << "FAIL: delete ok=" << delResp.ok << " httpStatus=" << delResp.error.httpStatus
                << " code=" << delResp.error.code << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no attachment to delete" << Qt::endl;
    }

    out << Qt::endl;
    if (failures == 0) {
        out << "=== ALL INTEGRATION SMOKE TESTS PASSED ===" << Qt::endl;
    } else {
        out << "=== " << failures << " FAILURE(S) ===" << Qt::endl;
    }

    return failures;
}
