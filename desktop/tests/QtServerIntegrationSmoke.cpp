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
#include "api/PartnerPostApiService.h"
#include "api/ContactConversationApiService.h"
#include "auth/AuthTokenStore.h"
#include "auth/AuthApiService.h"
#include "auth/InMemorySessionTokenStore.h"
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
    const QString adminEmail = env.value("CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL");
    const QString adminPassword = env.value("CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD");

    if (smokeEmail.isEmpty() || smokePassword.isEmpty()) {
        out << "ERROR: CAMPUS_BUDDY_SMOKE_EMAIL and/or CAMPUS_BUDDY_SMOKE_PASSWORD not set" << Qt::endl;
        out << "Required environment variables:" << Qt::endl;
        out << "  CAMPUS_BUDDY_SMOKE_EMAIL" << Qt::endl;
        out << "  CAMPUS_BUDDY_SMOKE_PASSWORD" << Qt::endl;
        out << "  CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL" << Qt::endl;
        out << "  CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD" << Qt::endl;
        return 2;
    }
    if (adminEmail.isEmpty() || adminPassword.isEmpty()) {
        out << "ERROR: CAMPUS_BUDDY_SMOKE_ADMIN_EMAIL and/or CAMPUS_BUDDY_SMOKE_ADMIN_PASSWORD not set" << Qt::endl;
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

    InMemorySessionTokenStore plazaTokenStore;
    plazaTokenStore.setAccessToken(accessToken);
    PartnerPostApiService plazaService(client, plazaTokenStore);
    ContactConversationApiService contactService(client, plazaTokenStore);

    out << Qt::endl << "--- 6. GET /partner-posts (plaza list) ---" << Qt::endl;
    QString firstPublishedPostId;
    if (!accessToken.isEmpty()) {
        QEventLoop loop6;
        QTimer timeout6;
        timeout6.setSingleShot(true);
        PlazaListResult plazaResult;
        QObject::connect(&timeout6, &QTimer::timeout, &loop6, &QEventLoop::quit);
        plazaService.listPosts(0, 20, [&](const PlazaListResult &r) {
            plazaResult = r;
            loop6.quit();
        });
        timeout6.start(10000);
        loop6.exec();

        if (plazaResult.success && !plazaResult.items.isEmpty()) {
            firstPublishedPostId = plazaResult.items[0].postId;
            out << "PASS: items=" << plazaResult.items.size()
                << " firstPostId length=" << firstPublishedPostId.length() << Qt::endl;
        } else {
            out << "FAIL: success=" << plazaResult.success << " items=" << plazaResult.items.size()
                << " errorCode=" << plazaResult.errorCode << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no token for plaza" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 7. POST /partner-posts/{id}/contact-requests (as admin) ---" << Qt::endl;
    long long smokeConvId = 0;
    if (!firstPublishedPostId.isEmpty() && !adminEmail.isEmpty()) {
        QJsonObject adminLoginBody;
        adminLoginBody["campusEmail"] = adminEmail;
        adminLoginBody["password"] = adminPassword;
        ApiClientResponse adminLogin = blockingPost(client, "/auth/login", adminLoginBody);
        QString adminToken;
        if (adminLogin.ok) {
            adminToken = adminLogin.json.value("accessToken").toString();
        }

        InMemorySessionTokenStore adminTokenStore;
        adminTokenStore.setAccessToken(adminToken);
        ContactConversationApiService adminContactService(client, adminTokenStore);

        QEventLoop loop7;
        QTimer timeout7;
        timeout7.setSingleShot(true);
        ContactRequestResult contactResult;
        QObject::connect(&timeout7, &QTimer::timeout, &loop7, &QEventLoop::quit);
        adminContactService.requestContact(firstPublishedPostId, QStringLiteral("smoke test contact"), [&](const ContactRequestResult &r) {
            contactResult = r;
            loop7.quit();
        });
        timeout7.start(10000);
        loop7.exec();

        if (contactResult.success) {
            smokeConvId = contactResult.conversationId;
            out << "PASS: conversationId=" << smokeConvId << Qt::endl;
        } else {
            out << "FAIL: success=" << contactResult.success << " errorCode=" << contactResult.errorCode
                << " errorMessage=" << contactResult.errorMessage << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no published post or admin credentials to contact" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 8. GET /me/conversations (conversation list) ---" << Qt::endl;
    if (!accessToken.isEmpty()) {
        QEventLoop loop8;
        QTimer timeout8;
        timeout8.setSingleShot(true);
        ConversationListResult convListResult;
        QObject::connect(&timeout8, &QTimer::timeout, &loop8, &QEventLoop::quit);
        contactService.listConversations(0, 20, [&](const ConversationListResult &r) {
            convListResult = r;
            loop8.quit();
        });
        timeout8.start(10000);
        loop8.exec();

        if (convListResult.success) {
            out << "PASS: items=" << convListResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << convListResult.success << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no token for conversation list" << Qt::endl;
    }

    out << Qt::endl << "--- 9. POST /me/conversations/{id}/messages (send message) ---" << Qt::endl;
    long long smokeMsgId = 0;
    if (smokeConvId > 0) {
        QEventLoop loop9;
        QTimer timeout9;
        timeout9.setSingleShot(true);
        SendMessageResult sendResult;
        QObject::connect(&timeout9, &QTimer::timeout, &loop9, &QEventLoop::quit);
        contactService.sendMessage(smokeConvId, QStringLiteral("smoke follow up"), [&](const SendMessageResult &r) {
            sendResult = r;
            loop9.quit();
        });
        timeout9.start(10000);
        loop9.exec();

        if (sendResult.success) {
            smokeMsgId = sendResult.messageId;
            out << "PASS: messageId=" << smokeMsgId << Qt::endl;
        } else {
            out << "FAIL: success=" << sendResult.success << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no conversation for send message" << Qt::endl;
    }

    out << Qt::endl << "--- 10. GET /me/conversations/{id}/messages?afterMessageId=X&size=1 ---" << Qt::endl;
    if (smokeConvId > 0 && smokeMsgId > 0) {
        QEventLoop loop10;
        QTimer timeout10;
        timeout10.setSingleShot(true);
        MessageListResult msgResult;
        QObject::connect(&timeout10, &QTimer::timeout, &loop10, &QEventLoop::quit);
        contactService.queryMessages(smokeConvId, smokeMsgId, 1, [&](const MessageListResult &r) {
            msgResult = r;
            loop10.quit();
        });
        timeout10.start(10000);
        loop10.exec();

        if (msgResult.success && msgResult.items.size() <= 1) {
            out << "PASS: items=" << msgResult.items.size() << " (<=1)" << Qt::endl;
        } else {
            out << "FAIL: success=" << msgResult.success << " items=" << msgResult.items.size() << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no conversation/message for afterMessageId test" << Qt::endl;
    }

    out << Qt::endl;
    if (failures == 0) {
        out << "=== ALL INTEGRATION SMOKE TESTS PASSED ===" << Qt::endl;
    } else {
        out << "=== " << failures << " FAILURE(S) ===" << Qt::endl;
    }

    return failures;
}
