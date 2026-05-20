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
class ReviewListEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private ConversationRepository conversationRepository;
    @Autowired private ConversationMessageRepository conversationMessageRepository;
    @Autowired private ReviewRepository reviewRepository;

    private static int counter = 0;

    @BeforeEach
    void cleanUp() {
        reviewRepository.deleteAll();
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
        return "review-list-" + (++counter) + "@campus.edu.cn";
    }

    @Test
    void givenListRequiresAuth() throws Exception {
        mockMvc.perform(get("/api/me/reviews/given"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void receivedListRequiresAuth() throws Exception {
        mockMvc.perform(get("/api/me/reviews/received"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void givenListReturnsOnlyOwnReviews() throws Exception {
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
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5,\"reviewTags\":[\"守时\"]}"))
                .andExpect(status().isCreated());

        mockMvc.perform(get("/api/me/reviews/given")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(1)))
                .andExpect(jsonPath("$.items[0].rating").value(5))
                .andExpect(jsonPath("$.items[0].revieweeId").value(userB.toString()))
                .andExpect(jsonPath("$.totalElements").value(1));
    }

    @Test
    void receivedListReturnsOnlyOwnReceivedReviews() throws Exception {
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
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":4}"))
                .andExpect(status().isCreated());

        mockMvc.perform(get("/api/me/reviews/received")
                        .header("Authorization", "Bearer " + tokenB))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(1)))
                .andExpect(jsonPath("$.items[0].rating").value(4))
                .andExpect(jsonPath("$.items[0].reviewerId").value(userA.toString()))
                .andExpect(jsonPath("$.totalElements").value(1));

        mockMvc.perform(get("/api/me/reviews/received")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(0)))
                .andExpect(jsonPath("$.totalElements").value(0));
    }

    @Test
    void givenListOrderByCreatedAtDesc() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Instant base = Instant.now();

        Conversation conv1 = createValidConversation(userA, userB, base);
        Conversation conv2 = createValidConversation(userA, userB, base.plusSeconds(100));

        reviewRepository.save(new Review(conv1.getId(), userA, userB, 3, null, base));
        reviewRepository.save(new Review(conv2.getId(), userA, userB, 5, null, base.plusSeconds(100)));

        mockMvc.perform(get("/api/me/reviews/given")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(2)))
                .andExpect(jsonPath("$.items[0].rating").value(5))
                .andExpect(jsonPath("$.items[1].rating").value(3));
    }

    @Test
    void givenListSupportsPagination() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();
        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();
        Instant base = Instant.now();

        for (int i = 0; i < 3; i++) {
            Conversation conv = createValidConversation(userA, userB, base.plusSeconds(i * 100L));
            reviewRepository.save(new Review(conv.getId(), userA, userB, i + 1, null, base.plusSeconds(i * 100L)));
        }

        mockMvc.perform(get("/api/me/reviews/given")
                        .header("Authorization", "Bearer " + tokenA)
                        .param("page", "0")
                        .param("size", "2"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(2)))
                .andExpect(jsonPath("$.totalElements").value(3))
                .andExpect(jsonPath("$.totalPages").value(2))
                .andExpect(jsonPath("$.page").value(0))
                .andExpect(jsonPath("$.size").value(2));
    }

    @Test
    void unverifiedUserCanQueryEmptyList() throws Exception {
        String emailA = uniqueEmail();
        String tokenA = registerAndLogin(emailA, "Str0ngP@ss1", "UserA");

        mockMvc.perform(get("/api/me/reviews/given")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(0)))
                .andExpect(jsonPath("$.totalElements").value(0));

        mockMvc.perform(get("/api/me/reviews/received")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(0)))
                .andExpect(jsonPath("$.totalElements").value(0));
    }

    @Test
    void responseDoesNotContainSensitiveFields() throws Exception {
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
                        .content("{\"conversationId\":" + conv.getId() + ",\"revieweeId\":\"" + userB + "\",\"rating\":5}"))
                .andExpect(status().isCreated());

        String receivedResponse = mockMvc.perform(get("/api/me/reviews/received")
                        .header("Authorization", "Bearer " + tokenB))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        assertDoesNotContain(receivedResponse, "campusEmail", "password", "authenticationStatus", "realName", "studentNumber");
    }

    private void assertDoesNotContain(String response, String... fields) {
        for (String field : fields) {
            if (response.contains(field)) {
                throw new AssertionError("Response should not contain field: " + field);
            }
        }
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
