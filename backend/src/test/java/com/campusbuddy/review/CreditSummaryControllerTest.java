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

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class CreditSummaryControllerTest {

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
        return "credit-ctrl-" + (++counter) + "@campus.edu.cn";
    }

    @Test
    void myCreditSummaryRequiresAuth() throws Exception {
        mockMvc.perform(get("/api/me/credit-summary"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void publicCreditSummaryRequiresAuth() throws Exception {
        mockMvc.perform(get("/api/users/00000000-0000-0000-0000-000000000001/credit-summary"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void myCreditSummaryReturnsOwnSummary() throws Exception {
        String emailA = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        UUID userA = userAccountRepository.findByCampusEmail(emailA).orElseThrow().getUserId();

        mockMvc.perform(get("/api/me/credit-summary")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.userId").value(userA.toString()))
                .andExpect(jsonPath("$.averageRating").value(3.5))
                .andExpect(jsonPath("$.realConversationCount").value(0))
                .andExpect(jsonPath("$.ratingSampleCount").value(6));
    }

    @Test
    void publicCreditSummaryReturnsOtherSummary() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();

        mockMvc.perform(get("/api/users/" + userB + "/credit-summary")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.userId").value(userB.toString()))
                .andExpect(jsonPath("$.averageRating").value(3.5))
                .andExpect(jsonPath("$.realConversationCount").value(0));
    }

    @Test
    void publicCreditSummaryDoesNotExposeSensitiveFields() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();

        String response = mockMvc.perform(get("/api/users/" + userB + "/credit-summary")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        assertFalse(response.contains("reviewerId"));
        assertFalse(response.contains("reviewerEmail"));
        assertFalse(response.contains("reviewerName"));
    }

    @Test
    void publicCreditSummaryDoesNotExposeDisputedReviewCount() throws Exception {
        String emailA = uniqueEmail();
        String emailB = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");
        registerAndLoginVerified(emailB, "Str0ngP@ss2", "UserB");

        UUID userB = userAccountRepository.findByCampusEmail(emailB).orElseThrow().getUserId();

        String response = mockMvc.perform(get("/api/users/" + userB + "/credit-summary")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        assertFalse(response.contains("disputedReviewCount"));
    }

    @Test
    void myCreditSummaryIncludesDisputedReviewCount() throws Exception {
        String emailA = uniqueEmail();
        String tokenA = registerAndLoginVerified(emailA, "Str0ngP@ss1", "UserA");

        mockMvc.perform(get("/api/me/credit-summary")
                        .header("Authorization", "Bearer " + tokenA))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.disputedReviewCount").value(0));
    }

    private void assertFalse(boolean condition) {
        if (condition) throw new AssertionError("Expected field not to be present in response");
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
