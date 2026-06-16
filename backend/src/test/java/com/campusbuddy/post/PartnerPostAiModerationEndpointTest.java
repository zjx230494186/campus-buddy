package com.campusbuddy.post;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.CampusEmailVerificationCodeSender;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.moderation.PostModerationClient;
import com.campusbuddy.moderation.PostModerationDecision;
import com.campusbuddy.moderation.PostModerationRequest;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Import;
import org.springframework.context.annotation.Primary;
import org.springframework.http.MediaType;
import org.springframework.test.context.TestPropertySource;
import org.springframework.test.web.servlet.MockMvc;

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneOffset;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicReference;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
@TestPropertySource(properties = {
        "campus-buddy.post-moderation.enabled=true",
        "campus-buddy.post-moderation.auto-approve-threshold=0.92",
        "campus-buddy.post-moderation.auto-reject-threshold=0.85"
})
class PartnerPostAiModerationEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private StubPostModerationClient moderationClient;

    @AfterEach
    void resetModerationClient() {
        moderationClient.nextDecision.set(new PostModerationDecision("NEEDS_HUMAN", 0.0, "default", null, null));
    }

    @Test
    void submitReviewAutoPublishesHighConfidenceApprovedPost() throws Exception {
        moderationClient.nextDecision.set(new PostModerationDecision("APPROVE", 0.96, "safe", null, null));
        String token = registerVerifiedAndLogin("ai-approve@campus.edu.cn", "Str0ngPassword!", "AIApprove");
        String postId = createFullDraft(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("PUBLISHED"))
                .andExpect(jsonPath("$.publishedAt").exists());
    }

    @Test
    void submitReviewAutoRejectsHighConfidenceRejectedPost() throws Exception {
        moderationClient.nextDecision.set(new PostModerationDecision(
                "REJECT", 0.91, "unsafe", "内容不符合平台发布规范，请修改后重新提交。", "SAFETY"));
        String token = registerVerifiedAndLogin("ai-reject@campus.edu.cn", "Str0ngPassword!", "AIReject");
        String postId = createFullDraft(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("REJECTED"))
                .andExpect(jsonPath("$.rejectReason").value("内容不符合平台发布规范，请修改后重新提交。"));
    }

    @Test
    void submitReviewKeepsPendingWhenAiNeedsHuman() throws Exception {
        moderationClient.nextDecision.set(new PostModerationDecision("NEEDS_HUMAN", 0.70, "unclear", null, null));
        String token = registerVerifiedAndLogin("ai-human@campus.edu.cn", "Str0ngPassword!", "AIHuman");
        String postId = createFullDraft(token);

        mockMvc.perform(post("/api/me/partner-posts/" + postId + "/submit-review")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId").value(postId))
                .andExpect(jsonPath("$.status").value("PENDING_REVIEW"));
    }

    private String createFullDraft(String token) throws Exception {
        String body = """
                {"sceneType":"STUDY","title":"AI Moderation Test","description":"Looking for a study buddy for finals",
                 "timeMode":"TEXT_PREFERENCE","timeText":"Weekends","locationText":"Main Library",
                 "participantCount":2,"targetRequirement":"GPA above 3.0","contactPreference":"Use in-app chat",
                 "tags":["study"],"scenePayload":{"studyGoal":"Pass final exam"}}
                """;
        String response = mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(body))
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
        @Bean @Primary StubPostModerationClient stubPostModerationClient() { return new StubPostModerationClient(); }
    }

    static class StubPostModerationClient implements PostModerationClient {
        final AtomicReference<PostModerationDecision> nextDecision = new AtomicReference<>(
                new PostModerationDecision("NEEDS_HUMAN", 0.0, "default", null, null));

        @Override
        public PostModerationDecision moderate(PostModerationRequest request) {
            return nextDecision.get();
        }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();
        @Override public void send(String campusEmail, String verificationCode, String purpose) { codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode); }
        String latestCode(String campusEmail, String purpose) { return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase()); }
    }
}
