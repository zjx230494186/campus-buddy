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
class PartnerPostStudentLifecycleEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository partnerPostRepository;

    @Test
    void submitReviewRequiresAuthentication() throws Exception {
        String token = registerVerifiedAndLogin("lc-auth@campus.edu.cn", "Str0ngPassword!", "LCAuth");
        String postId = createEmptyDraft_helper(token);
        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .contentType(MediaType.APPLICATION_JSON))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void submitReviewRequiresVerifiedStatus() throws Exception {
        String email = "lc-unverified@campus.edu.cn";
        String token = registerAndLogin(email, "Str0ngPassword!", "LCUnverified");

        String ownerToken = registerVerifiedAndLogin("lc-verified-for-draft@campus.edu.cn", "Str0ngPassword!", "LCVFD");
        String postId = createDraftWithFullFields_helper(ownerToken);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("AUTHENTICATION_STATUS_REQUIRED"));
    }

    @Test
    void submitReviewOtherUsersDraftReturnsPostNotFound() throws Exception {
        String ownerToken = registerVerifiedAndLogin("lc-owner@campus.edu.cn", "Str0ngPassword!", "LCOwner");
        String postId = createDraftWithFullFields_helper(ownerToken);

        String otherToken = registerVerifiedAndLogin("lc-other@campus.edu.cn", "Str0ngPassword!", "LCOther");

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + otherToken))
                .andExpect(status().isNotFound())
                .andExpect(jsonPath("$.code").value("POST_NOT_FOUND"));
    }

    @Test
    void submitReviewWithCompleteFieldsSucceeds() throws Exception {
        String token = registerVerifiedAndLogin("lc-submit@campus.edu.cn", "Str0ngPassword!", "LCSubmit");
        String postId = createDraftWithFullFields_helper(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("PENDING_REVIEW"))
                .andExpect(jsonPath("$.allowedActions", hasItem("WITHDRAW_REVIEW")));
    }

    @Test
    void submitReviewMissingPublicRequiredFieldReturnsValidationFailed() throws Exception {
        String token = registerVerifiedAndLogin("lc-missing@campus.edu.cn", "Str0ngPassword!", "LCMissing");
        String postId = createEmptyDraft_helper(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details.title").exists());
    }

    @Test
    void submitReviewMissingSceneRequiredFieldReturnsValidationFailed() throws Exception {
        String token = registerVerifiedAndLogin("lc-sceneval@campus.edu.cn", "Str0ngPassword!", "LCSceneVal");
        String body = """
                {"sceneType":"MEAL","title":"Test","description":"Desc","timeMode":"TEXT_PREFERENCE",
                 "timeText":"Weekends","locationText":"Lib","participantCount":2,
                 "targetRequirement":"GPA>3","contactPreference":"WeChat ID"}
                """;
        String postId = createDraftWithBody_helper(token, body);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details['scenePayload.canteen']").exists());
    }

    @Test
    void submitReviewWithPhoneNumberInContactPreferenceReturnsValidationFailed() throws Exception {
        String token = registerVerifiedAndLogin("lc-phone@campus.edu.cn", "Str0ngPassword!", "LCPhone");
        String body = """
                {"sceneType":"STUDY","title":"Test","description":"Desc","timeMode":"TEXT_PREFERENCE",
                 "timeText":"Weekends","locationText":"Lib","participantCount":2,
                 "targetRequirement":"GPA>3","contactPreference":"Call me at 13812345678",
                 "scenePayload":{"studyGoal":"pass exam"}}
                """;
        String postId = createDraftWithBody_helper(token, body);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details.contactPreference").exists());
    }

    @Test
    void submitReviewNonDraftReturnsPostStatusConflict() throws Exception {
        String token = registerVerifiedAndLogin("lc-conflict@campus.edu.cn", "Str0ngPassword!", "LCConflict");
        String postId = createDraftWithFullFields_helper(token);

        java.util.UUID pid = java.util.UUID.fromString(postId);
        PartnerPost post = partnerPostRepository.findById(pid).orElseThrow();
        post.setStatus("PENDING_REVIEW");
        partnerPostRepository.save(post);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("POST_STATUS_CONFLICT"));
    }

    @Test
    void withdrawReviewFromPendingReviewSucceeds() throws Exception {
        String token = registerVerifiedAndLogin("lc-withdraw@campus.edu.cn", "Str0ngPassword!", "LCWithdraw");
        String postId = createDraftWithFullFields_helper(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk());

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/withdraw-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("DRAFT"))
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")))
                .andExpect(jsonPath("$.allowedActions", hasItem("SUBMIT_REVIEW")));
    }

    @Test
    void withdrawReviewNonPendingReviewReturnsPostStatusConflict() throws Exception {
        String token = registerVerifiedAndLogin("lc-wdconflict@campus.edu.cn", "Str0ngPassword!", "LCWDConflict");
        String postId = createDraftWithFullFields_helper(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/withdraw-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("POST_STATUS_CONFLICT"));
    }

    @Test
    void unpublishFromPublishedSucceeds() throws Exception {
        String token = registerVerifiedAndLogin("lc-unpub@campus.edu.cn", "Str0ngPassword!", "LCUnpub");
        String postId = createDraftWithFullFields_helper(token);

        java.util.UUID pid = java.util.UUID.fromString(postId);
        PartnerPost post = partnerPostRepository.findById(pid).orElseThrow();
        post.setStatus("PUBLISHED");
        post.setPublishedAt(Instant.now());
        partnerPostRepository.save(post);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/unpublish")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("DRAFT"))
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")))
                .andExpect(jsonPath("$.allowedActions", hasItem("SUBMIT_REVIEW")));
    }

    @Test
    void unpublishNonPublishedReturnsPostStatusConflict() throws Exception {
        String token = registerVerifiedAndLogin("lc-unpubconf@campus.edu.cn", "Str0ngPassword!", "LCUnpubConf");
        String postId = createDraftWithFullFields_helper(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/unpublish")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("POST_STATUS_CONFLICT"));
    }

    @Test
    void allowedActionsChangeWithStatus() throws Exception {
        String token = registerVerifiedAndLogin("lc-actions@campus.edu.cn", "Str0ngPassword!", "LCActions");
        String postId = createDraftWithFullFields_helper(token);

        mockMvc.perform(get("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")))
                .andExpect(jsonPath("$.allowedActions", hasItem("SUBMIT_REVIEW")))
                .andExpect(jsonPath("$.allowedActions", not(hasItem("WITHDRAW_REVIEW"))));

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk());

        mockMvc.perform(get("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.allowedActions", hasItem("WITHDRAW_REVIEW")))
                .andExpect(jsonPath("$.allowedActions", not(hasItem("EDIT"))));

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/withdraw-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk());

        mockMvc.perform(get("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")))
                .andExpect(jsonPath("$.allowedActions", hasItem("SUBMIT_REVIEW")));
    }

    @Test
    void rejectedStatusHasEditAndViewRejectReason() throws Exception {
        String token = registerVerifiedAndLogin("lc-rejected@campus.edu.cn", "Str0ngPassword!", "LCRejected");
        String postId = createDraftWithFullFields_helper(token);

        java.util.UUID pid = java.util.UUID.fromString(postId);
        PartnerPost post = partnerPostRepository.findById(pid).orElseThrow();
        post.setStatus("REJECTED");
        post.setRejectReason("Content does not meet guidelines");
        partnerPostRepository.save(post);

        mockMvc.perform(get("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("REJECTED"))
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")))
                .andExpect(jsonPath("$.allowedActions", hasItem("VIEW_REJECT_REASON")))
                .andExpect(jsonPath("$.rejectReason").value("Content does not meet guidelines"));
    }

    private String createDraftWithFullFields_helper(String token) throws Exception {
        String body = """
                {"sceneType":"STUDY","title":"Study Partner Wanted","description":"Looking for a study buddy for finals",
                 "timeMode":"TEXT_PREFERENCE","timeText":"Weekends and evenings",
                 "locationText":"Main Library 3F","participantCount":2,
                 "targetRequirement":"GPA above 3.0","contactPreference":"WeChat campus_buddy_2026",
                 "tags":["math","finals"],"scenePayload":{"studyGoal":"Pass final exam"}}
                """;
        return createDraftWithBody_helper(token, body);
    }

    private String createDraftWithBody_helper(String token, String body) throws Exception {
        String response = mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(body))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.postId");
    }

    private String createEmptyDraft_helper(String token) throws Exception {
        String response = mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.postId");
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

    private String registerAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
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
