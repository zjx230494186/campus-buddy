package com.campusbuddy.post;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.CampusEmailVerificationCodeSender;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Import;
import org.springframework.context.annotation.Primary;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneOffset;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import static org.hamcrest.Matchers.*;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class PartnerPostAdminReviewEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository partnerPostRepository;

    @Test
    void reviewQueueRequiresAuthentication() throws Exception {
        mockMvc.perform(get("/api/admin/partner-posts/review-queue"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void reviewQueueRequiresAdminRole() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-student@campus.edu.cn", "Str0ngPassword!", "ARStudent");

        mockMvc.perform(get("/api/admin/partner-posts/review-queue")
                        .header("Authorization", "Bearer " + studentToken))
                .andExpect(status().isForbidden());
    }

    @Test
    void adminCanViewReviewQueueWithOnlyPendingReview() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-queue@campus.edu.cn", "Str0ngPassword!", "ARQueue");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-queue@campus.edu.cn", "Str0ngPassword!", "ARAdminQueue");

        mockMvc.perform(get("/api/admin/partner-posts/review-queue")
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))))
                .andExpect(jsonPath("$.items[?(@.postId=='" + postId + "')].status").value(hasItem("PENDING_REVIEW")));
    }

    @Test
    void adminDetailReturnsPostFieldsAndPublisherSummary() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-detail@campus.edu.cn", "Str0ngPassword!", "ARDetail");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-detail@campus.edu.cn", "Str0ngPassword!", "ARAdminDetail");

        mockMvc.perform(get("/api/admin/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("PENDING_REVIEW"))
                .andExpect(jsonPath("$.publisherDisplayName").exists())
                .andExpect(jsonPath("$.publisherAuthenticationStatus").exists());
    }

    @Test
    void adminDetailExcludesSensitiveFields() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-nosens@campus.edu.cn", "Str0ngPassword!", "ARNosens");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-nosens@campus.edu.cn", "Str0ngPassword!", "ARAdminNosens");

        String response = mockMvc.perform(get("/api/admin/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        for (String field : new String[]{"campusEmail", "studentNumber", "realName", "objectKey"}) {
            org.junit.jupiter.api.Assertions.assertFalse(
                    response.contains("\"" + field + "\""),
                    "Response must not contain field: " + field
            );
        }
    }

    @Test
    void adminApproveChangesStatusToPublished() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-approve@campus.edu.cn", "Str0ngPassword!", "ARApprove");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-approve@campus.edu.cn", "Str0ngPassword!", "ARAdminApprove");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVE\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("PUBLISHED"))
                .andExpect(jsonPath("$.reviewedBy").exists())
                .andExpect(jsonPath("$.reviewedAt").exists())
                .andExpect(jsonPath("$.publishedAt").exists());
    }

    @Test
    void adminRejectChangesStatusToRejected() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-reject@campus.edu.cn", "Str0ngPassword!", "ARReject");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-reject@campus.edu.cn", "Str0ngPassword!", "ARAdminReject");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"REJECT\",\"reason\":\"Content violates policy\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("REJECTED"))
                .andExpect(jsonPath("$.rejectReason").value("Content violates policy"))
                .andExpect(jsonPath("$.reviewedBy").exists())
                .andExpect(jsonPath("$.reviewedAt").exists())
                .andExpect(jsonPath("$.publishedAt").value(nullValue()));
    }

    @Test
    void rejectWithoutReasonReturnsValidationFailed() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-noreas@campus.edu.cn", "Str0ngPassword!", "ARNoreas");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-noreas@campus.edu.cn", "Str0ngPassword!", "ARAdminNoreas");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"REJECT\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details.reason").exists());
    }

    @Test
    void invalidDecisionReturnsValidationFailed() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-invdec@campus.edu.cn", "Str0ngPassword!", "ARInvdec");
        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-invdec@campus.edu.cn", "Str0ngPassword!", "ARAdminInvdec");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"INVALID\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details.decision").exists());
    }

    @Test
    void reviewNonPendingReviewReturnsPostStatusConflict() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-nonpend@campus.edu.cn", "Str0ngPassword!", "ARNonpend");
        String postId = createFullDraft(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-nonpend@campus.edu.cn", "Str0ngPassword!", "ARAdminNonpend");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVE\"}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("POST_STATUS_CONFLICT"));
    }

    @Test
    void reviewNonexistentPostReturnsPostNotFound() throws Exception {
        String adminToken = registerAdminAndLogin("ar-admin-nf@campus.edu.cn", "Str0ngPassword!", "ARAdminNF");

        mockMvc.perform(post("/api/admin/partner-posts/00000000-0000-0000-0000-000000000000/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVE\"}"))
                .andExpect(status().isNotFound())
                .andExpect(jsonPath("$.code").value("POST_NOT_FOUND"));
    }

    @Test
    void approveExceedsPublishedLimitReturnsError() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-limit@campus.edu.cn", "Str0ngPassword!", "ARLimit");
        java.util.UUID publisherId = getUserIdFromEmail("ar-limit@campus.edu.cn");

        for (int i = 0; i < 10; i++) {
            PartnerPost post = new PartnerPost(publisherId, "PUBLISHED", Instant.now());
            post.setTitle("Published " + i);
            post.setSceneType("STUDY");
            post.setPublishedAt(Instant.now());
            partnerPostRepository.save(post);
        }

        String postId = createFullDraftAndSubmit(studentToken);

        String adminToken = registerAdminAndLogin("ar-admin-limit@campus.edu.cn", "Str0ngPassword!", "ARAdminLimit");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVE\"}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("PUBLISHED_POST_LIMIT_EXCEEDED"));
    }

    @Test
    void withdrawnPostThenAdminReviewReturnsPostStatusConflict() throws Exception {
        String studentToken = registerVerifiedAndLogin("ar-withdrawn@campus.edu.cn", "Str0ngPassword!", "ARWithdrawn");
        String postId = createFullDraftAndSubmit(studentToken);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/withdraw-review")
                        .header("Authorization", "Bearer " + studentToken))
                .andExpect(status().isOk());

        String adminToken = registerAdminAndLogin("ar-admin-withdrawn@campus.edu.cn", "Str0ngPassword!", "ARAdminWithdrawn");

        mockMvc.perform(post("/api/admin/partner-posts/" + postId + "/review")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVE\"}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("POST_STATUS_CONFLICT"));
    }

    private java.util.UUID getUserIdFromEmail(String email) {
        return userAccountRepository.findByCampusEmail(email)
                .map(UserAccount::getUserId)
                .orElseThrow();
    }

    private String createFullDraft(String token) throws Exception {
        String body = """
                {"sceneType":"STUDY","title":"Admin Review Test","description":"For admin review testing",
                 "timeMode":"TEXT_PREFERENCE","timeText":"Weekends","locationText":"Library",
                 "participantCount":2,"targetRequirement":"GPA>3","contactPreference":"WeChat campus_buddy",
                 "tags":["test"],"scenePayload":{"studyGoal":"Pass exam"}}
                """;
        String response = mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(body))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.postId");
    }

    private String createFullDraftAndSubmit(String token) throws Exception {
        String postId = createFullDraft(token);
        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk());
        return postId;
    }

    private String registerVerifiedAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("VERIFIED");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String registerAdminAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("VERIFIED");
        account.setAccountRole("ADMIN");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String login(String email, String password) throws Exception {
        String response = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"password\":\"%s\"}".formatted(email, password)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.accessToken");
    }

    private String verifiedTicket(String email) throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"purpose\":\"REGISTER_OR_LOGIN\"}".formatted(email)))
                .andExpect(status().isOk());
        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");
        String response = mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"code\":\"%s\",\"purpose\":\"REGISTER_OR_LOGIN\"}".formatted(email, code)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.verificationTicket");
    }

    @TestConfiguration
    static class TestConfig {
        @Bean @Primary CapturingCampusEmailVerificationCodeSender capturingSender() { return new CapturingCampusEmailVerificationCodeSender(); }
        @Bean @Primary Clock testClock() { return Clock.fixed(Instant.parse("2026-05-23T00:00:00Z"), ZoneOffset.UTC); }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();
        @Override public void send(String campusEmail, String verificationCode, String purpose) { codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode); }
        String latestCode(String campusEmail, String purpose) { return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase()); }
    }
}
