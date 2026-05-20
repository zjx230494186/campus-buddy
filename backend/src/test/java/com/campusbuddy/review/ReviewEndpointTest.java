package com.campusbuddy.review;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.CampusEmailVerificationCodeSender;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.contact.*;
import com.jayway.jsonpath.JsonPath;
import org.junit.jupiter.api.BeforeEach;
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

import java.time.Instant;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

import static org.hamcrest.Matchers.*;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class ReviewEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private ConversationRepository conversationRepository;
    @Autowired private ConversationMessageRepository conversationMessageRepository;
    @Autowired private ContactUnlockRecordRepository contactUnlockRecordRepository;
    @Autowired private ReviewRepository reviewRepository;

    private static int counter = 0;

    @BeforeEach
    void cleanUp() {
        reviewRepository.deleteAll();
        contactUnlockRecordRepository.deleteAll();
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();
    }

    private String registerAndLoginVerified(String email, String password, String displayName) throws Exception {
        String token = registerAndLogin(email, password, displayName);
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("VERIFIED");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String registerAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {"campusEmail":"%s","verificationTicket":"%s","password":"%s","displayName":"%s"}
                                """.formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        return login(email, password);
    }

    private String login(String email, String password) throws Exception {
        String response = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {"campusEmail":"%s","password":"%s"}
                                """.formatted(email, password)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return JsonPath.read(response, "$.accessToken");
    }

    private String verifiedTicket(String email) throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {"campusEmail":"%s","purpose":"REGISTER_OR_LOGIN"}
                                """.formatted(email)))
                .andExpect(status().isOk());

        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");
        String response = mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {"campusEmail":"%s","code":"%s","purpose":"REGISTER_OR_LOGIN"}
                                """.formatted(email, code)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return JsonPath.read(response, "$.verificationTicket");
    }

    private String uniqueEmail() {
        return "review-test-" + (++counter) + "@campus.edu.cn";
    }

    @Test
    void verifiedUserCanSubmit5StarReview() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        String tokenB = registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5,\"reviewTags\":[\"守时\"]}"))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.rating").value(5))
                .andExpect(jsonPath("$.status").value("ACTIVE"))
                .andExpect(jsonPath("$.modifiedOnce").value(false));
    }

    @Test
    void normalConversationRejects6Star() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":6}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void unlockedConversationAllows6Star() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Instant now = Instant.now();
        Conversation conv = createValidConversation(userA, userB, now);
        contactUnlockRecordRepository.save(new ContactUnlockRecord(conv.getId(), now));

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":6}"))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.rating").value(6));
    }

    @Test
    void duplicateReviewRejected() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        String body = "{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5}";
        mockMvc.perform(post("/api/me/reviews").header("Authorization", "Bearer " + tokenA).contentType(MediaType.APPLICATION_JSON).content(body))
                .andExpect(status().isCreated());

        mockMvc.perform(post("/api/me/reviews").header("Authorization", "Bearer " + tokenA).contentType(MediaType.APPLICATION_JSON).content(body))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("REVIEW_ALREADY_EXISTS"));
    }

    @Test
    void nonPresetTagRejected() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5,\"reviewTags\":[\"自定义标签\"]}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void reviewWithoutTagsSucceeds() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":4}"))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.reviewTags", hasSize(0)));
    }

    @Test
    void modifyWithin24HoursSucceeds() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        String createResult = mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5,\"reviewTags\":[\"守时\"]}"))
                .andExpect(status().isCreated())
                .andReturn().getResponse().getContentAsString();

        long reviewId = new com.fasterxml.jackson.databind.ObjectMapper().readTree(createResult).get("id").asLong();

        mockMvc.perform(put("/api/me/reviews/" + reviewId)
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"rating\":3,\"reviewTags\":[\"迟到\"]}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.rating").value(3))
                .andExpect(jsonPath("$.modifiedOnce").value(true));
    }

    @Test
    void secondModificationRejected() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        String createResult = mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5}"))
                .andExpect(status().isCreated())
                .andReturn().getResponse().getContentAsString();

        long reviewId = new com.fasterxml.jackson.databind.ObjectMapper().readTree(createResult).get("id").asLong();

        mockMvc.perform(put("/api/me/reviews/" + reviewId)
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"rating\":3}"))
                .andExpect(status().isOk());

        mockMvc.perform(put("/api/me/reviews/" + reviewId)
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"rating\":4}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("REVIEW_MODIFICATION_LIMIT_EXCEEDED"));
    }

    @Test
    void revieweeNotOtherParticipantRejected() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String emailC = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");
        registerAndLoginVerified(emailC, "Str0ngP@ss3", "UserC");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        UUID userC = userAccountRepository.findByCampusEmail(emailC).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userC + "\",\"rating\":5}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("CONVERSATION_NOT_REVIEWABLE"));
    }

    @Test
    void insufficientMessagesRejected() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Instant now = Instant.now();
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", now));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", now));

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("CONVERSATION_NOT_REVIEWABLE"));
    }

    @Test
    void unverifiedUserSubmitReviewReturnsAuthStatusRequired() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLogin(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5}"))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("AUTHENTICATION_STATUS_REQUIRED"));
    }

    @Test
    void unauthenticatedSubmitReviewReturns401() throws Exception {
        mockMvc.perform(post("/api/me/reviews")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":1,\"revieweeId\":\"00000000-0000-0000-0000-000000000001\",\"rating\":5}"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void otherUserModifyReviewReturnsNotFound() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String emailC = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");
        String tokenC = registerAndLoginVerified(emailC, "Str0ngP@ss3", "UserC");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Conversation conv = createValidConversation(userA, userB, Instant.now());

        String createResult = mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + tokenA)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5}"))
                .andExpect(status().isCreated())
                .andReturn().getResponse().getContentAsString();

        long reviewId = new com.fasterxml.jackson.databind.ObjectMapper().readTree(createResult).get("id").asLong();

        mockMvc.perform(put("/api/me/reviews/" + reviewId)
                        .header("Authorization", "Bearer " + tokenC)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"rating\":3}"))
                .andExpect(status().isNotFound())
                .andExpect(jsonPath("$.code").value("REVIEW_NOT_FOUND"));
    }

    private Conversation createValidConversation(UUID p1, UUID p2, Instant now) {
        Conversation conv = conversationRepository.save(new Conversation(p1, p2, "ACTIVE", now));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p1, "USER_TEXT", now));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p1, "USER_TEXT", now.plusSeconds(1)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p2, "USER_TEXT", now.plusSeconds(2)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p2, "USER_TEXT", now.plusSeconds(3)));
        return conv;
    }

    @TestConfiguration
    static class TestConfig {

        @Bean
        @Primary
        CapturingCampusEmailVerificationCodeSender capturingSender() {
            return new CapturingCampusEmailVerificationCodeSender();
        }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();

        @Override
        public void send(String campusEmail, String verificationCode, String purpose) {
            codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode);
        }

        String latestCode(String campusEmail, String purpose) {
            return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase());
        }
    }
}
