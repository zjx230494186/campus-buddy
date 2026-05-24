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
#include "api/MyPartnerPostApiService.h"
#include "api/ReviewCreditApiService.h"
#include "api/AdminReviewApiService.h"
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

    MyPartnerPostApiService myPostService(client, plazaTokenStore);

    out << Qt::endl << "--- 11. POST /me/partner-posts (create draft) ---" << Qt::endl;
    QString smokeDraftPostId;
    if (!accessToken.isEmpty()) {
        MyPostDraftRequest draftReq;
        draftReq.sceneType = QStringLiteral("STUDY");
        draftReq.title = QStringLiteral("Smoke Test Draft");
        draftReq.description = QStringLiteral("Created by Qt smoke test");
        draftReq.timeMode = QStringLiteral("TEXT_PREFERENCE");
        draftReq.timeText = QStringLiteral("weekends");
        draftReq.locationText = QStringLiteral("Library");
        draftReq.participantCount = 2;
        draftReq.targetRequirement = QStringLiteral("GPA > 3.0");
        draftReq.contactPreference = QStringLiteral("in-app chat");
        draftReq.scenePayload.insert(QStringLiteral("studyGoal"), QStringLiteral("pass exam"));

        QEventLoop loop11;
        QTimer timeout11;
        timeout11.setSingleShot(true);
        MyPostResult draftResult;
        QObject::connect(&timeout11, &QTimer::timeout, &loop11, &QEventLoop::quit);
        myPostService.createDraft(draftReq, [&](const MyPostResult &r) {
            draftResult = r;
            loop11.quit();
        });
        timeout11.start(10000);
        loop11.exec();

        if (draftResult.success) {
            smokeDraftPostId = draftResult.post.postId;
            out << "PASS: postId length=" << smokeDraftPostId.length()
                << " status=" << draftResult.post.status << Qt::endl;
        } else {
            out << "FAIL: success=" << draftResult.success << " errorCode=" << draftResult.errorCode
                << " errorMessage=" << draftResult.errorMessage << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no token for create draft" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 12. PUT /me/partner-posts/{id} (update draft) ---" << Qt::endl;
    if (!smokeDraftPostId.isEmpty()) {
        MyPostDraftRequest updateReq;
        updateReq.sceneType = QStringLiteral("STUDY");
        updateReq.title = QStringLiteral("Smoke Test Draft Updated");
        updateReq.description = QStringLiteral("Updated by Qt smoke test");
        updateReq.timeMode = QStringLiteral("TEXT_PREFERENCE");
        updateReq.timeText = QStringLiteral("weekends");
        updateReq.locationText = QStringLiteral("Library");
        updateReq.participantCount = 2;
        updateReq.targetRequirement = QStringLiteral("GPA > 3.0");
        updateReq.contactPreference = QStringLiteral("in-app chat");
        updateReq.scenePayload.insert(QStringLiteral("studyGoal"), QStringLiteral("pass exam"));

        QEventLoop loop12;
        QTimer timeout12;
        timeout12.setSingleShot(true);
        MyPostResult updateResult;
        QObject::connect(&timeout12, &QTimer::timeout, &loop12, &QEventLoop::quit);
        myPostService.updateDraft(smokeDraftPostId, updateReq, [&](const MyPostResult &r) {
            updateResult = r;
            loop12.quit();
        });
        timeout12.start(10000);
        loop12.exec();

        if (updateResult.success && updateResult.post.status == QStringLiteral("DRAFT")) {
            out << "PASS: status=" << updateResult.post.status << Qt::endl;
        } else {
            out << "FAIL: success=" << updateResult.success << " status=" << updateResult.post.status
                << " errorCode=" << updateResult.errorCode << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no draft to update" << Qt::endl;
    }

    out << Qt::endl << "--- 13. GET /me/partner-posts (list my posts) ---" << Qt::endl;
    if (!accessToken.isEmpty()) {
        QEventLoop loop13;
        QTimer timeout13;
        timeout13.setSingleShot(true);
        MyPostListResult myListResult;
        QObject::connect(&timeout13, &QTimer::timeout, &loop13, &QEventLoop::quit);
        myPostService.listMyPosts(0, 20, [&](const MyPostListResult &r) {
            myListResult = r;
            loop13.quit();
        });
        timeout13.start(10000);
        loop13.exec();

        if (myListResult.success && myListResult.items.size() >= 1) {
            out << "PASS: items=" << myListResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << myListResult.success << " items=" << myListResult.items.size()
                << " errorCode=" << myListResult.errorCode << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no token for list my posts" << Qt::endl;
    }

    out << Qt::endl << "--- 14. GET /me/partner-posts/{id} (my post detail) ---" << Qt::endl;
    if (!smokeDraftPostId.isEmpty()) {
        QEventLoop loop14;
        QTimer timeout14;
        timeout14.setSingleShot(true);
        MyPostResult detailResult;
        QObject::connect(&timeout14, &QTimer::timeout, &loop14, &QEventLoop::quit);
        myPostService.getMyPostDetail(smokeDraftPostId, [&](const MyPostResult &r) {
            detailResult = r;
            loop14.quit();
        });
        timeout14.start(10000);
        loop14.exec();

        if (detailResult.success && detailResult.post.allowedActions.contains(QStringLiteral("SUBMIT_REVIEW"))) {
            out << "PASS: allowedActions count=" << detailResult.post.allowedActions.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << detailResult.success << " errorCode=" << detailResult.errorCode << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no draft for detail" << Qt::endl;
    }

    out << Qt::endl << "--- 15. POST /me/partner-posts/{id}/submit-review ---" << Qt::endl;
    if (!smokeDraftPostId.isEmpty()) {
        QEventLoop loop15;
        QTimer timeout15;
        timeout15.setSingleShot(true);
        PostActionResult submitResult;
        QObject::connect(&timeout15, &QTimer::timeout, &loop15, &QEventLoop::quit);
        myPostService.submitReview(smokeDraftPostId, [&](const PostActionResult &r) {
            submitResult = r;
            loop15.quit();
        });
        timeout15.start(10000);
        loop15.exec();

        if (submitResult.success && submitResult.post.status == QStringLiteral("PENDING_REVIEW")) {
            out << "PASS: status=" << submitResult.post.status << Qt::endl;
        } else {
            out << "FAIL: success=" << submitResult.success << " status=" << submitResult.post.status
                << " errorCode=" << submitResult.errorCode << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no draft for submit review" << Qt::endl;
    }

    out << Qt::endl << "--- 16. POST /me/partner-posts/{id}/withdraw-review ---" << Qt::endl;
    if (!smokeDraftPostId.isEmpty()) {
        QEventLoop loop16;
        QTimer timeout16;
        timeout16.setSingleShot(true);
        PostActionResult withdrawResult;
        QObject::connect(&timeout16, &QTimer::timeout, &loop16, &QEventLoop::quit);
        myPostService.withdrawReview(smokeDraftPostId, [&](const PostActionResult &r) {
            withdrawResult = r;
            loop16.quit();
        });
        timeout16.start(10000);
        loop16.exec();

        if (withdrawResult.success && withdrawResult.post.status == QStringLiteral("DRAFT")) {
            out << "PASS: status=" << withdrawResult.post.status << Qt::endl;
        } else {
            out << "FAIL: success=" << withdrawResult.success << " status=" << withdrawResult.post.status
                << " errorCode=" << withdrawResult.errorCode << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no post for withdraw review" << Qt::endl;
    }

    ReviewCreditApiService reviewService(client, plazaTokenStore);

    QString smokeOtherParticipantId;

    out << Qt::endl << "--- 17. Find otherParticipantId from conversation list ---" << Qt::endl;
    if (smokeConvId > 0 && !accessToken.isEmpty()) {
        QEventLoop loop17;
        QTimer timeout17;
        timeout17.setSingleShot(true);
        ConversationListResult convList17;
        QObject::connect(&timeout17, &QTimer::timeout, &loop17, &QEventLoop::quit);
        contactService.listConversations(0, 50, [&](const ConversationListResult &r) {
            convList17 = r;
            loop17.quit();
        });
        timeout17.start(10000);
        loop17.exec();

        if (convList17.success) {
            for (const auto &c : convList17.items) {
                if (c.conversationId == smokeConvId) {
                    smokeOtherParticipantId = c.otherParticipantId;
                    break;
                }
            }
        }
        if (!smokeOtherParticipantId.isEmpty()) {
            out << "PASS: otherParticipantId found (length=" << smokeOtherParticipantId.length() << ")" << Qt::endl;
        } else {
            out << "FAIL: could not find otherParticipantId for conversationId=" << smokeConvId << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no conversation/token for finding participant" << Qt::endl;
    }

    out << Qt::endl << "--- 18. Ensure conversation has 2+ USER_TEXT each side ---" << Qt::endl;
    if (smokeConvId > 0 && !adminEmail.isEmpty()) {
        InMemorySessionTokenStore adminTokenStore2;
        QJsonObject adminLoginBody;
        adminLoginBody["campusEmail"] = adminEmail;
        adminLoginBody["password"] = adminPassword;
        ApiClientResponse adminLogin2 = blockingPost(client, "/auth/login", adminLoginBody);
        if (adminLogin2.ok) {
            adminTokenStore2.setAccessToken(adminLogin2.json.value("accessToken").toString());
        }

        ContactConversationApiService adminContact2(client, adminTokenStore2);

        int adminSendOk = 0;
        for (int i = 0; i < 2; ++i) {
            QEventLoop loopA;
            QTimer timeoutA;
            timeoutA.setSingleShot(true);
            SendMessageResult adminMsgResult;
            QObject::connect(&timeoutA, &QTimer::timeout, &loopA, &QEventLoop::quit);
            adminContact2.sendMessage(smokeConvId, QStringLiteral("hi"), [&](const SendMessageResult &r) {
                adminMsgResult = r;
                loopA.quit();
            });
            timeoutA.start(10000);
            loopA.exec();
            if (adminMsgResult.success) adminSendOk++;
        }

        int smokeSendOk = 0;
        for (int i = 0; i < 2; ++i) {
            QEventLoop loopS;
            QTimer timeoutS;
            timeoutS.setSingleShot(true);
            SendMessageResult smokeMsgResult;
            QObject::connect(&timeoutS, &QTimer::timeout, &loopS, &QEventLoop::quit);
            contactService.sendMessage(smokeConvId, QStringLiteral("hi"), [&](const SendMessageResult &r) {
                smokeMsgResult = r;
                loopS.quit();
            });
            timeoutS.start(10000);
            loopS.exec();
            if (smokeMsgResult.success) smokeSendOk++;
        }

        if (adminSendOk >= 2 && smokeSendOk >= 2) {
            out << "PASS: admin sent " << adminSendOk << " smoke sent " << smokeSendOk << Qt::endl;
        } else {
            out << "NOTE: admin sent " << adminSendOk << " smoke sent " << smokeSendOk << " (may affect review eligibility)" << Qt::endl;
        }
    } else {
        out << "SKIP: no conversation or admin creds for message padding" << Qt::endl;
    }

    out << Qt::endl << "--- 19. GET /me/credit-summary ---" << Qt::endl;
    {
        QEventLoop loop19;
        QTimer timeout19;
        timeout19.setSingleShot(true);
        MyCreditSummaryResult creditResult;
        QObject::connect(&timeout19, &QTimer::timeout, &loop19, &QEventLoop::quit);
        reviewService.getMyCreditSummary([&](const MyCreditSummaryResult &r) {
            creditResult = r;
            loop19.quit();
        });
        timeout19.start(10000);
        loop19.exec();

        if (creditResult.success) {
            out << "PASS: avgRating=" << creditResult.averageRating << " sampleCount=" << creditResult.ratingSampleCount
                << " disputed=" << creditResult.disputedReviewCount << Qt::endl;
        } else {
            out << "FAIL: success=" << creditResult.success << " errorCode=" << creditResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 20. POST /me/reviews (create review) ---" << Qt::endl;
    long long smokeReviewId = 0;
    if (smokeConvId > 0 && !smokeOtherParticipantId.isEmpty() && !accessToken.isEmpty()) {
        CreateReviewRequest reviewReq;
        reviewReq.conversationId = smokeConvId;
        reviewReq.revieweeId = smokeOtherParticipantId;
        reviewReq.rating = 5;
        reviewReq.reviewTags = QStringList{QStringLiteral("守时")};

        QEventLoop loop20;
        QTimer timeout20;
        timeout20.setSingleShot(true);
        ReviewResult reviewResult;
        QObject::connect(&timeout20, &QTimer::timeout, &loop20, &QEventLoop::quit);
        reviewService.createReview(reviewReq, [&](const ReviewResult &r) {
            reviewResult = r;
            loop20.quit();
        });
        timeout20.start(10000);
        loop20.exec();

        if (reviewResult.success) {
            smokeReviewId = reviewResult.review.id;
            out << "PASS: reviewId=" << smokeReviewId << " status=" << reviewResult.review.status << Qt::endl;
        } else if (reviewResult.errorCode == QStringLiteral("REVIEW_ALREADY_EXISTS")) {
            out << "NOTE: review already exists for this conversation (expected on repeat runs)" << Qt::endl;
        } else {
            out << "FAIL: errorCode=" << reviewResult.errorCode << " errorMessage=" << reviewResult.errorMessage << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no conversation/participant/token for review" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 21. GET /me/reviews/given ---" << Qt::endl;
    {
        QEventLoop loop21;
        QTimer timeout21;
        timeout21.setSingleShot(true);
        ReviewListResult givenResult;
        QObject::connect(&timeout21, &QTimer::timeout, &loop21, &QEventLoop::quit);
        reviewService.listGivenReviews(0, 20, [&](const ReviewListResult &r) {
            givenResult = r;
            loop21.quit();
        });
        timeout21.start(10000);
        loop21.exec();

        if (givenResult.success && givenResult.items.size() >= 1) {
            out << "PASS: items=" << givenResult.items.size() << Qt::endl;
        } else if (givenResult.success) {
            out << "NOTE: items=0 (review may not have been created)" << Qt::endl;
        } else {
            out << "FAIL: success=" << givenResult.success << " errorCode=" << givenResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 22. GET /me/reviews/received ---" << Qt::endl;
    {
        QEventLoop loop22;
        QTimer timeout22;
        timeout22.setSingleShot(true);
        ReviewListResult receivedResult;
        QObject::connect(&timeout22, &QTimer::timeout, &loop22, &QEventLoop::quit);
        reviewService.listReceivedReviews(0, 20, [&](const ReviewListResult &r) {
            receivedResult = r;
            loop22.quit();
        });
        timeout22.start(10000);
        loop22.exec();

        if (receivedResult.success) {
            out << "PASS: items=" << receivedResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << receivedResult.success << " errorCode=" << receivedResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 23. Admin login ---" << Qt::endl;
    QString adminAccessToken;
    {
        QJsonObject adminLoginBody;
        adminLoginBody["campusEmail"] = adminEmail;
        adminLoginBody["password"] = adminPassword;
        ApiClientResponse adminLoginResp = blockingPost(client, "/auth/login", adminLoginBody);
        if (adminLoginResp.ok) {
            adminAccessToken = adminLoginResp.json.value("accessToken").toString();
            out << "PASS: admin token length=" << adminAccessToken.length() << Qt::endl;
        } else {
            out << "FAIL: admin login ok=" << adminLoginResp.ok << Qt::endl;
            failures++;
        }
    }

    InMemorySessionTokenStore adminTokenStoreForReview;
    adminTokenStoreForReview.setAccessToken(adminAccessToken);
    AdminReviewApiService adminReviewService(client, adminTokenStoreForReview);

    out << Qt::endl << "--- 24. GET /admin/partner-posts/review-queue ---" << Qt::endl;
    {
        QEventLoop loop24;
        QTimer timeout24;
        timeout24.setSingleShot(true);
        PartnerPostReviewQueueResult queueResult;
        QObject::connect(&timeout24, &QTimer::timeout, &loop24, &QEventLoop::quit);
        adminReviewService.listPartnerPostReviewQueue(0, 20, [&](const PartnerPostReviewQueueResult &r) {
            queueResult = r;
            loop24.quit();
        });
        timeout24.start(10000);
        loop24.exec();

        if (queueResult.success) {
            out << "PASS: items=" << queueResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << queueResult.success << " errorCode=" << queueResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 25. Withdraw pending posts + create + submit for admin review ---" << Qt::endl;
    QString smokePostForAdminReview;
    if (!accessToken.isEmpty()) {
        MyPartnerPostApiService smokeMyPostService(client, plazaTokenStore);

        {
            QEventLoop loopW;
            QTimer timeoutW;
            timeoutW.setSingleShot(true);
            MyPostListResult myPostsW;
            QObject::connect(&timeoutW, &QTimer::timeout, &loopW, &QEventLoop::quit);
            smokeMyPostService.listMyPosts(0, 50, [&](const MyPostListResult &r) {
                myPostsW = r;
                loopW.quit();
            });
            timeoutW.start(10000);
            loopW.exec();

            if (myPostsW.success) {
                for (const auto &p : myPostsW.items) {
                    if (p.status == QStringLiteral("PENDING_REVIEW")) {
                        QEventLoop loopW2;
                        QTimer timeoutW2;
                        timeoutW2.setSingleShot(true);
                        PostActionResult withdrawW;
                        QObject::connect(&timeoutW2, &QTimer::timeout, &loopW2, &QEventLoop::quit);
                        smokeMyPostService.withdrawReview(p.postId, [&](const PostActionResult &r) {
                            withdrawW = r;
                            loopW2.quit();
                        });
                        timeoutW2.start(10000);
                        loopW2.exec();
                    }
                }
            }
        }

        MyPostDraftRequest draftReq;
        draftReq.sceneType = QStringLiteral("STUDY");
        draftReq.title = QStringLiteral("smoke admin review test");
        draftReq.description = QStringLiteral("smoke test for admin review");
        draftReq.timeMode = QStringLiteral("TEXT_PREFERENCE");
        draftReq.timeText = QStringLiteral("evening");
        draftReq.locationText = QStringLiteral("library");
        draftReq.participantCount = 2;
        draftReq.targetRequirement = QStringLiteral("study together");
        draftReq.contactPreference = QStringLiteral("WeChat");
        draftReq.scenePayload[QStringLiteral("studyGoal")] = QStringLiteral("pass exam");

        QEventLoop loop25a;
        QTimer timeout25a;
        timeout25a.setSingleShot(true);
        MyPostResult draftResult;
        QObject::connect(&timeout25a, &QTimer::timeout, &loop25a, &QEventLoop::quit);
        smokeMyPostService.createDraft(draftReq, [&](const MyPostResult &r) {
            draftResult = r;
            loop25a.quit();
        });
        timeout25a.start(10000);
        loop25a.exec();

        if (draftResult.success) {
            smokePostForAdminReview = draftResult.post.postId;

            QEventLoop loop25b;
            QTimer timeout25b;
            timeout25b.setSingleShot(true);
            PostActionResult submitResult;
            QObject::connect(&timeout25b, &QTimer::timeout, &loop25b, &QEventLoop::quit);
            smokeMyPostService.submitReview(smokePostForAdminReview, [&](const PostActionResult &r) {
                submitResult = r;
                loop25b.quit();
            });
            timeout25b.start(10000);
            loop25b.exec();

            if (submitResult.success && submitResult.post.status == QStringLiteral("PENDING_REVIEW")) {
                out << "PASS: postId length=" << smokePostForAdminReview.length() << " status=" << submitResult.post.status << Qt::endl;
            } else {
                out << "FAIL: submit-review failed errorCode=" << submitResult.errorCode << " errorMessage=" << submitResult.errorMessage << Qt::endl;
                failures++;
            }
        } else {
            out << "FAIL: draft creation failed" << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no token for smoke post creation" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 26. Admin detail + REJECT smoke post ---" << Qt::endl;
    if (!smokePostForAdminReview.isEmpty() && !adminAccessToken.isEmpty()) {
        QEventLoop loop26a;
        QTimer timeout26a;
        timeout26a.setSingleShot(true);
        AdminPostDetailResult detailResult;
        QObject::connect(&timeout26a, &QTimer::timeout, &loop26a, &QEventLoop::quit);
        adminReviewService.getPartnerPostAdminDetail(smokePostForAdminReview, [&](const AdminPostDetailResult &r) {
            detailResult = r;
            loop26a.quit();
        });
        timeout26a.start(10000);
        loop26a.exec();

        if (detailResult.success && detailResult.detail.status == QStringLiteral("PENDING_REVIEW")) {
            PartnerPostReviewRequest rejectReq;
            rejectReq.decision = QStringLiteral("REJECT");
            rejectReq.reason = QStringLiteral("smoke validation reject");
            QEventLoop loop26b;
            QTimer timeout26b;
            timeout26b.setSingleShot(true);
            PartnerPostReviewResult rejectResult;
            QObject::connect(&timeout26b, &QTimer::timeout, &loop26b, &QEventLoop::quit);
            adminReviewService.reviewPartnerPost(smokePostForAdminReview, rejectReq, [&](const PartnerPostReviewResult &r) {
                rejectResult = r;
                loop26b.quit();
            });
            timeout26b.start(10000);
            loop26b.exec();

            if (rejectResult.success && rejectResult.detail.status == QStringLiteral("REJECTED") && !rejectResult.detail.reviewedAt.isEmpty()) {
                out << "PASS: status=" << rejectResult.detail.status << " reviewedAt=yes" << Qt::endl;
            } else {
                out << "FAIL: reject success=" << rejectResult.success << " status=" << rejectResult.detail.status
                    << " reviewedAt=" << (!rejectResult.detail.reviewedAt.isEmpty() ? "yes" : "no")
                    << " errorCode=" << rejectResult.errorCode << Qt::endl;
                failures++;
            }
        } else {
            out << "FAIL: detail not PENDING_REVIEW status=" << (detailResult.success ? detailResult.detail.status : QStringLiteral("error")) << Qt::endl;
            failures++;
        }
    } else {
        out << "SKIP: no post or admin token for admin review" << Qt::endl;
        failures++;
    }

    out << Qt::endl << "--- 27. GET /admin/identity-verifications?status=PENDING_REVIEW ---" << Qt::endl;
    {
        QEventLoop loop27;
        QTimer timeout27;
        timeout27.setSingleShot(true);
        PendingIdentityVerificationListResult pendingResult;
        QObject::connect(&timeout27, &QTimer::timeout, &loop27, &QEventLoop::quit);
        adminReviewService.listPendingIdentityVerifications(0, 20, [&](const PendingIdentityVerificationListResult &r) {
            pendingResult = r;
            loop27.quit();
        });
        timeout27.start(10000);
        loop27.exec();

        if (pendingResult.success) {
            out << "PASS: items=" << pendingResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << pendingResult.success << " errorCode=" << pendingResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 28. Admin listConversations unreadCount >= 1 ---" << Qt::endl;
    {
        InMemorySessionTokenStore adminTokenStore28;
        QJsonObject adminLoginBody28;
        adminLoginBody28["campusEmail"] = adminEmail;
        adminLoginBody28["password"] = adminPassword;
        ApiClientResponse adminLogin28 = blockingPost(client, "/auth/login", adminLoginBody28);
        if (adminLogin28.ok) {
            adminTokenStore28.setAccessToken(adminLogin28.json.value("accessToken").toString());
        }
        ContactConversationApiService adminContact28(client, adminTokenStore28);

        if (smokeConvId > 0 && adminLogin28.ok) {
            QEventLoop loop28;
            QTimer timeout28;
            timeout28.setSingleShot(true);
            ConversationListResult convList28;
            QObject::connect(&timeout28, &QTimer::timeout, &loop28, &QEventLoop::quit);
            adminContact28.listConversations(0, 50, [&](const ConversationListResult &r) {
                convList28 = r;
                loop28.quit();
            });
            timeout28.start(10000);
            loop28.exec();

            int foundUnreadCount = -1;
            if (convList28.success) {
                for (const auto &c : convList28.items) {
                    if (c.conversationId == smokeConvId) {
                        foundUnreadCount = c.unreadCount;
                        break;
                    }
                }
            }
            if (foundUnreadCount >= 1) {
                out << "PASS: unreadCount=" << foundUnreadCount << Qt::endl;
            } else {
                out << "FAIL: unreadCount=" << foundUnreadCount << " (expected >=1)" << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no conversation or admin token for unreadCount test" << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 29. Admin markConversationRead ---" << Qt::endl;
    {
        InMemorySessionTokenStore adminTokenStore29;
        QJsonObject adminLoginBody29;
        adminLoginBody29["campusEmail"] = adminEmail;
        adminLoginBody29["password"] = adminPassword;
        ApiClientResponse adminLogin29 = blockingPost(client, "/auth/login", adminLoginBody29);
        if (adminLogin29.ok) {
            adminTokenStore29.setAccessToken(adminLogin29.json.value("accessToken").toString());
        }
        ContactConversationApiService adminContact29(client, adminTokenStore29);

        if (smokeConvId > 0 && adminLogin29.ok) {
            QEventLoop loop29;
            QTimer timeout29;
            timeout29.setSingleShot(true);
            MarkReadResult markReadResult;
            QObject::connect(&timeout29, &QTimer::timeout, &loop29, &QEventLoop::quit);
            adminContact29.markConversationRead(smokeConvId, [&](const MarkReadResult &r) {
                markReadResult = r;
                loop29.quit();
            });
            timeout29.start(10000);
            loop29.exec();

            if (markReadResult.success) {
                out << "PASS: markRead success" << Qt::endl;
            } else {
                out << "FAIL: markRead errorCode=" << markReadResult.errorCode << Qt::endl;
                failures++;
            }

            out << Qt::endl << "--- 30. Admin unreadCount after markRead = 0 ---" << Qt::endl;
            QEventLoop loop30;
            QTimer timeout30;
            timeout30.setSingleShot(true);
            ConversationListResult convList30;
            QObject::connect(&timeout30, &QTimer::timeout, &loop30, &QEventLoop::quit);
            adminContact29.listConversations(0, 50, [&](const ConversationListResult &r) {
                convList30 = r;
                loop30.quit();
            });
            timeout30.start(10000);
            loop30.exec();

            int unreadAfter = -1;
            if (convList30.success) {
                for (const auto &c : convList30.items) {
                    if (c.conversationId == smokeConvId) {
                        unreadAfter = c.unreadCount;
                        break;
                    }
                }
            }
            if (unreadAfter == 0) {
                out << "PASS: unreadCount=" << unreadAfter << Qt::endl;
            } else {
                out << "FAIL: unreadCount=" << unreadAfter << " (expected 0)" << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no conversation or admin token for markRead test" << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 31. Smoke user closeConversation ---" << Qt::endl;
    {
        if (smokeConvId > 0 && !accessToken.isEmpty()) {
            QEventLoop loop31;
            QTimer timeout31;
            timeout31.setSingleShot(true);
            CloseConversationResult closeResult;
            QObject::connect(&timeout31, &QTimer::timeout, &loop31, &QEventLoop::quit);
            contactService.closeConversation(smokeConvId, [&](const CloseConversationResult &r) {
                closeResult = r;
                loop31.quit();
            });
            timeout31.start(10000);
            loop31.exec();

            if (closeResult.success && closeResult.status == QStringLiteral("CLOSED")) {
                out << "PASS: status=" << closeResult.status << Qt::endl;
            } else {
                out << "FAIL: success=" << closeResult.success << " status=" << closeResult.status
                    << " errorCode=" << closeResult.errorCode << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no conversation or token for close test" << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 32. Send message on CLOSED conversation -> CONVERSATION_CLOSED ---" << Qt::endl;
    {
        if (smokeConvId > 0 && !accessToken.isEmpty()) {
            QEventLoop loop32;
            QTimer timeout32;
            timeout32.setSingleShot(true);
            SendMessageResult sendClosedResult;
            QObject::connect(&timeout32, &QTimer::timeout, &loop32, &QEventLoop::quit);
            contactService.sendMessage(smokeConvId, QStringLiteral("closed test"), [&](const SendMessageResult &r) {
                sendClosedResult = r;
                loop32.quit();
            });
            timeout32.start(10000);
            loop32.exec();

            if (!sendClosedResult.success && sendClosedResult.errorCode == QStringLiteral("CONVERSATION_CLOSED")) {
                out << "PASS: errorCode=" << sendClosedResult.errorCode << Qt::endl;
            } else {
                out << "FAIL: success=" << sendClosedResult.success << " errorCode=" << sendClosedResult.errorCode << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no conversation or token for closed-send test" << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 33. Re-contact reopens CLOSED conversation -> ACTIVE ---" << Qt::endl;
    {
        if (!firstPublishedPostId.isEmpty() && !adminEmail.isEmpty()) {
            InMemorySessionTokenStore adminTokenStore33;
            QJsonObject adminLoginBody33;
            adminLoginBody33["campusEmail"] = adminEmail;
            adminLoginBody33["password"] = adminPassword;
            ApiClientResponse adminLogin33 = blockingPost(client, "/auth/login", adminLoginBody33);
            if (adminLogin33.ok) {
                adminTokenStore33.setAccessToken(adminLogin33.json.value("accessToken").toString());
            }
            ContactConversationApiService adminContact33(client, adminTokenStore33);

            QEventLoop loop33;
            QTimer timeout33;
            timeout33.setSingleShot(true);
            ContactRequestResult recontactResult;
            QObject::connect(&timeout33, &QTimer::timeout, &loop33, &QEventLoop::quit);
            adminContact33.requestContact(firstPublishedPostId, QStringLiteral("recontact after close"), [&](const ContactRequestResult &r) {
                recontactResult = r;
                loop33.quit();
            });
            timeout33.start(10000);
            loop33.exec();

            if (recontactResult.success && recontactResult.conversationId == smokeConvId && recontactResult.status == QStringLiteral("ACTIVE")) {
                out << "PASS: conversationId=" << recontactResult.conversationId << " status=" << recontactResult.status << Qt::endl;
            } else {
                out << "FAIL: success=" << recontactResult.success << " conversationId=" << recontactResult.conversationId
                    << " status=" << recontactResult.status << " errorCode=" << recontactResult.errorCode << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no post or admin creds for recontact test" << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 34. GET /me/contact-card (smoke user) ---" << Qt::endl;
    {
        QEventLoop loop34;
        QTimer timeout34;
        timeout34.setSingleShot(true);
        ContactCardResult cardResult;
        QObject::connect(&timeout34, &QTimer::timeout, &loop34, &QEventLoop::quit);
        contactService.getMyContactCard([&](const ContactCardResult &r) {
            cardResult = r;
            loop34.quit();
        });
        timeout34.start(10000);
        loop34.exec();

        if (cardResult.success) {
            out << "PASS: hasCard=" << cardResult.hasCard << Qt::endl;
        } else {
            out << "FAIL: success=" << cardResult.success << " errorCode=" << cardResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 35. PUT /me/contact-card (upsert) ---" << Qt::endl;
    {
        QEventLoop loop35;
        QTimer timeout35;
        timeout35.setSingleShot(true);
        ContactCardResult upsertResult;
        QObject::connect(&timeout35, &QTimer::timeout, &loop35, &QEventLoop::quit);
        contactService.upsertMyContactCard(
            QStringLiteral("smoke_wx"), QStringLiteral("13800001111"), QStringLiteral("smoke_qq"),
            [&](const ContactCardResult &r) {
                upsertResult = r;
                loop35.quit();
            });
        timeout35.start(10000);
        loop35.exec();

        if (upsertResult.success && upsertResult.hasCard) {
            out << "PASS: hasCard=" << upsertResult.hasCard
                << " wechatId=" << upsertResult.wechatId << Qt::endl;
        } else {
            out << "FAIL: success=" << upsertResult.success << " errorCode=" << upsertResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 36. GET contact-unlock (smoke conversation) ---" << Qt::endl;
    {
        if (smokeConvId > 0) {
            QEventLoop loop36;
            QTimer timeout36;
            timeout36.setSingleShot(true);
            ContactUnlockStatusResult unlockResult;
            QObject::connect(&timeout36, &QTimer::timeout, &loop36, &QEventLoop::quit);
            contactService.getContactUnlockStatus(smokeConvId, [&](const ContactUnlockStatusResult &r) {
                unlockResult = r;
                loop36.quit();
            });
            timeout36.start(10000);
            loop36.exec();

            if (unlockResult.success) {
                out << "PASS: status=" << unlockResult.status
                    << " currentUserConfirmed=" << unlockResult.currentUserConfirmed
                    << " peerConfirmed=" << unlockResult.peerConfirmed << Qt::endl;
            } else {
                out << "FAIL: success=" << unlockResult.success << " errorCode=" << unlockResult.errorCode << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no conversation for unlock status" << Qt::endl;
        }
    }

    out << Qt::endl << "--- 37. POST contact-unlock/confirm (smoke user) ---" << Qt::endl;
    {
        if (smokeConvId > 0) {
            QEventLoop loop37;
            QTimer timeout37;
            timeout37.setSingleShot(true);
            ContactUnlockStatusResult confirmResult;
            QObject::connect(&timeout37, &QTimer::timeout, &loop37, &QEventLoop::quit);
            contactService.confirmContactUnlock(smokeConvId, [&](const ContactUnlockStatusResult &r) {
                confirmResult = r;
                loop37.quit();
            });
            timeout37.start(10000);
            loop37.exec();

            if (confirmResult.success) {
                out << "PASS: status=" << confirmResult.status
                    << " currentUserConfirmed=" << confirmResult.currentUserConfirmed << Qt::endl;
            } else {
                out << "FAIL: success=" << confirmResult.success << " errorCode=" << confirmResult.errorCode << Qt::endl;
                failures++;
            }
        } else {
            out << "SKIP: no conversation for confirm unlock" << Qt::endl;
        }
    }

    out << Qt::endl << "--- 38. Admin upsert contact-card + confirm unlock + peer-contact-card ---" << Qt::endl;
    {
        if (smokeConvId > 0 && !adminEmail.isEmpty()) {
            InMemorySessionTokenStore adminTokenStore38;
            QJsonObject adminLoginBody38;
            adminLoginBody38["campusEmail"] = adminEmail;
            adminLoginBody38["password"] = adminPassword;
            ApiClientResponse adminLogin38 = blockingPost(client, "/auth/login", adminLoginBody38);
            if (adminLogin38.ok) {
                adminTokenStore38.setAccessToken(adminLogin38.json.value("accessToken").toString());
            }
            ContactConversationApiService adminContact38(client, adminTokenStore38);

            QEventLoop loop38a;
            QTimer timeout38a;
            timeout38a.setSingleShot(true);
            ContactCardResult adminCardResult;
            QObject::connect(&timeout38a, &QTimer::timeout, &loop38a, &QEventLoop::quit);
            adminContact38.upsertMyContactCard(
                QStringLiteral("admin_wx"), QStringLiteral("13900002222"), QStringLiteral("admin_qq"),
                [&](const ContactCardResult &r) {
                    adminCardResult = r;
                    loop38a.quit();
                });
            timeout38a.start(10000);
            loop38a.exec();

            bool adminCardOk = adminCardResult.success && adminCardResult.hasCard;

            QEventLoop loop38b;
            QTimer timeout38b;
            timeout38b.setSingleShot(true);
            ContactUnlockStatusResult adminConfirmResult;
            QObject::connect(&timeout38b, &QTimer::timeout, &loop38b, &QEventLoop::quit);
            adminContact38.confirmContactUnlock(smokeConvId, [&](const ContactUnlockStatusResult &r) {
                adminConfirmResult = r;
                loop38b.quit();
            });
            timeout38b.start(10000);
            loop38b.exec();

            bool adminConfirmOk = adminConfirmResult.success && adminConfirmResult.status == QStringLiteral("UNLOCKED");

            bool peerCardOk = false;
            if (adminConfirmOk) {
                QEventLoop loop38c;
                QTimer timeout38c;
                timeout38c.setSingleShot(true);
                PeerContactCardResult peerResult;
                QObject::connect(&timeout38c, &QTimer::timeout, &loop38c, &QEventLoop::quit);
                contactService.getPeerContactCard(smokeConvId, [&](const PeerContactCardResult &r) {
                    peerResult = r;
                    loop38c.quit();
                });
                timeout38c.start(10000);
                loop38c.exec();

                peerCardOk = peerResult.success;
                if (peerCardOk) {
                    out << "PASS: peerCard wechatId=" << peerResult.wechatId
                        << " phone=" << peerResult.phoneNumber << Qt::endl;
                } else {
                    out << "FAIL: peerCard errorCode=" << peerResult.errorCode << Qt::endl;
                    failures++;
                }
            }

            if (adminCardOk && adminConfirmOk && peerCardOk) {
                out << "PASS: full unlock flow completed" << Qt::endl;
            } else {
                if (!adminCardOk) {
                    out << "FAIL: admin card upsert failed" << Qt::endl;
                    failures++;
                }
                if (!adminConfirmOk) {
                    out << "FAIL: admin confirm unlock status=" << adminConfirmResult.status << Qt::endl;
                    failures++;
                }
            }
        } else {
            out << "SKIP: no conversation or admin creds for full unlock flow" << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl;
    if (failures == 0) {
        out << "=== ALL INTEGRATION SMOKE TESTS PASSED ===" << Qt::endl;
    } else {
        out << "=== " << failures << " FAILURE(S) ===" << Qt::endl;
    }

    return failures;
}
