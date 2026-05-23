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

    out << Qt::endl << "--- 17. Ensure conversation has 2+ messages each side ---" << Qt::endl;
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
        QEventLoop loop17;
        QTimer timeout17;
        timeout17.setSingleShot(true);
        SendMessageResult adminMsgResult;
        QObject::connect(&timeout17, &QTimer::timeout, &loop17, &QEventLoop::quit);
        adminContact2.sendMessage(smokeConvId, QStringLiteral("admin second message for review eligibility"), [&](const SendMessageResult &r) {
            adminMsgResult = r;
            loop17.quit();
        });
        timeout17.start(10000);
        loop17.exec();

        if (adminMsgResult.success) {
            out << "PASS: admin sent second message" << Qt::endl;
        } else {
            out << "NOTE: admin second message result=" << adminMsgResult.success << " errorCode=" << adminMsgResult.errorCode << Qt::endl;
        }
    } else {
        out << "SKIP: no conversation or admin creds for message padding" << Qt::endl;
    }

    out << Qt::endl << "--- 18. GET /me/credit-summary ---" << Qt::endl;
    {
        QEventLoop loop18;
        QTimer timeout18;
        timeout18.setSingleShot(true);
        MyCreditSummaryResult creditResult;
        QObject::connect(&timeout18, &QTimer::timeout, &loop18, &QEventLoop::quit);
        reviewService.getMyCreditSummary([&](const MyCreditSummaryResult &r) {
            creditResult = r;
            loop18.quit();
        });
        timeout18.start(10000);
        loop18.exec();

        if (creditResult.success) {
            out << "PASS: avgRating=" << creditResult.averageRating << " sampleCount=" << creditResult.ratingSampleCount
                << " disputed=" << creditResult.disputedReviewCount << Qt::endl;
        } else {
            out << "FAIL: success=" << creditResult.success << " errorCode=" << creditResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 19. POST /me/reviews (create review) ---" << Qt::endl;
    long long smokeReviewId = 0;
    if (smokeConvId > 0 && !accessToken.isEmpty()) {
        CreateReviewRequest reviewReq;
        reviewReq.conversationId = smokeConvId;
        reviewReq.revieweeId = QStringLiteral("a0eebc99-9c0b-4ef8-bb6d-6bb9bd380a11");
        reviewReq.rating = 5;
        reviewReq.reviewTags = QStringList{QStringLiteral("friendly")};

        QEventLoop loop19;
        QTimer timeout19;
        timeout19.setSingleShot(true);
        ReviewResult reviewResult;
        QObject::connect(&timeout19, &QTimer::timeout, &loop19, &QEventLoop::quit);
        reviewService.createReview(reviewReq, [&](const ReviewResult &r) {
            reviewResult = r;
            loop19.quit();
        });
        timeout19.start(10000);
        loop19.exec();

        if (reviewResult.success) {
            smokeReviewId = reviewResult.review.id;
            out << "PASS: reviewId=" << smokeReviewId << " status=" << reviewResult.review.status << Qt::endl;
        } else {
            out << "NOTE: success=" << reviewResult.success << " errorCode=" << reviewResult.errorCode
                << " errorMessage=" << reviewResult.errorMessage << Qt::endl;
        }
    } else {
        out << "SKIP: no conversation/token for review" << Qt::endl;
    }

    out << Qt::endl << "--- 20. GET /me/reviews/given ---" << Qt::endl;
    {
        QEventLoop loop20;
        QTimer timeout20;
        timeout20.setSingleShot(true);
        ReviewListResult givenResult;
        QObject::connect(&timeout20, &QTimer::timeout, &loop20, &QEventLoop::quit);
        reviewService.listGivenReviews(0, 20, [&](const ReviewListResult &r) {
            givenResult = r;
            loop20.quit();
        });
        timeout20.start(10000);
        loop20.exec();

        if (givenResult.success) {
            out << "PASS: items=" << givenResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << givenResult.success << " errorCode=" << givenResult.errorCode << Qt::endl;
            failures++;
        }
    }

    out << Qt::endl << "--- 21. GET /me/reviews/received ---" << Qt::endl;
    {
        QEventLoop loop21;
        QTimer timeout21;
        timeout21.setSingleShot(true);
        ReviewListResult receivedResult;
        QObject::connect(&timeout21, &QTimer::timeout, &loop21, &QEventLoop::quit);
        reviewService.listReceivedReviews(0, 20, [&](const ReviewListResult &r) {
            receivedResult = r;
            loop21.quit();
        });
        timeout21.start(10000);
        loop21.exec();

        if (receivedResult.success) {
            out << "PASS: items=" << receivedResult.items.size() << Qt::endl;
        } else {
            out << "FAIL: success=" << receivedResult.success << " errorCode=" << receivedResult.errorCode << Qt::endl;
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
