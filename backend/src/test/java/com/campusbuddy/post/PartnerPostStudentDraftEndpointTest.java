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
class PartnerPostStudentDraftEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository partnerPostRepository;

    @Test
    void createDraftRequiresAuthentication() throws Exception {
        mockMvc.perform(post("/api/me/partner-posts")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{}"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void createDraftRequiresVerifiedStatus() throws Exception {
        String email = "pp-unverified@campus.edu.cn";
        String token = registerAndLogin(email, "Str0ngPassword!", "PPUnverified");

        mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{}"))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("AUTHENTICATION_STATUS_REQUIRED"));
    }

    @Test
    void verifiedUserCanCreateMinimalDraft() throws Exception {
        String token = registerVerifiedAndLogin("pp-create@campus.edu.cn", "Str0ngPassword!", "PPCreate");

        mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.postId", not(emptyOrNullString())))
                .andExpect(jsonPath("$.status").value("DRAFT"))
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")));
    }

    @Test
    void createDraftWithInvalidSceneTypeReturnsValidationFailed() throws Exception {
        String token = registerVerifiedAndLogin("pp-scene@campus.edu.cn", "Str0ngPassword!", "PPScene");

        mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"sceneType\":\"INVALID_TYPE\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details.sceneType").exists());
    }

    @Test
    void createDraftWithTooLongTitleReturnsValidationFailed() throws Exception {
        String token = registerVerifiedAndLogin("pp-title@campus.edu.cn", "Str0ngPassword!", "PPTitle");
        String longTitle = "a".repeat(41);

        mockMvc.perform(post("/api/me/partner-posts")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"title\":\"" + longTitle + "\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.details.title").exists());
    }

    @Test
    void ownerCanUpdateOwnDraft() throws Exception {
        String token = registerVerifiedAndLogin("pp-update@campus.edu.cn", "Str0ngPassword!", "PPUpdate");
        String postId = createDraft(token, "{}");

        mockMvc.perform(put("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"title\":\"Updated Title\",\"locationText\":\"Library\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.title").value("Updated Title"))
                .andExpect(jsonPath("$.locationText").value("Library"));
    }

    @Test
    void cannotUpdateOtherUsersDraft() throws Exception {
        String ownerToken = registerVerifiedAndLogin("pp-owner@campus.edu.cn", "Str0ngPassword!", "PPOwner");
        String postId = createDraft(ownerToken, "{}");

        String otherToken = registerVerifiedAndLogin("pp-other@campus.edu.cn", "Str0ngPassword!", "PPOther");

        mockMvc.perform(put("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + otherToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"title\":\"Hack\"}"))
                .andExpect(status().isNotFound());
    }

    @Test
    void updateNonDraftReturnsPostStatusConflict() throws Exception {
        String email = "pp-nondraft@campus.edu.cn";
        String token = registerVerifiedAndLogin(email, "Str0ngPassword!", "PPNonDraft");

        String postId = createDraft(token, "{}");
        java.util.UUID pid = java.util.UUID.fromString(postId);
        PartnerPost post = partnerPostRepository.findById(pid).orElseThrow();
        post.setStatus("PENDING_REVIEW");
        partnerPostRepository.save(post);

        mockMvc.perform(put("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"title\":\"Try Update\"}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("POST_STATUS_CONFLICT"));
    }

    @Test
    void myListReturnsOnlyOwnDrafts() throws Exception {
        String tokenA = registerVerifiedAndLogin("pp-lista@campus.edu.cn", "Str0ngPassword!", "PPListA");
        createDraft(tokenA, "{\"title\":\"A's draft\"}");

        String tokenB = registerVerifiedAndLogin("pp-listb@campus.edu.cn", "Str0ngPassword!", "PPListB");
        createDraft(tokenB, "{\"title\":\"B's draft\"}");

        mockMvc.perform(get("/api/me/partner-posts?status=DRAFT")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(1)))
                .andExpect(jsonPath("$.items[0].title").value("A's draft"));
    }

    @Test
    void myDetailReturnsAllowedActionsAndExcludesSensitiveFields() throws Exception {
        String token = registerVerifiedAndLogin("pp-detail@campus.edu.cn", "Str0ngPassword!", "PPDetail");
        String postId = createDraft(token, "{}");

        String response = mockMvc.perform(get("/api/me/partner-posts/" + postId)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.allowedActions", hasItem("EDIT")))
                .andReturn().getResponse().getContentAsString();

        assertThatJsonDoesNotContainFields(response, "campusEmail", "studentNumber", "realName", "objectKey");
    }

    private void assertThatJsonDoesNotContainFields(String json, String... fields) {
        for (String field : fields) {
            org.junit.jupiter.api.Assertions.assertFalse(
                    json.contains("\"" + field + "\""),
                    "Response must not contain field: " + field
            );
        }
    }

    private String createDraft(String token, String body) throws Exception {
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
