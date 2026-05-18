package com.campusbuddy.auth;

import com.campusbuddy.TestcontainersConfiguration;
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

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneOffset;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

import static org.hamcrest.Matchers.*;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class IdentityVerificationEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private CapturingCampusEmailVerificationCodeSender codeSender;

    @Autowired
    private UserAccountRepository userAccountRepository;

    @Autowired
    private IdentityVerificationSubmissionRepository submissionRepository;

    @Test
    void submitRequiresAuthentication() throws Exception {
        mockMvc.perform(post("/api/auth/identity-verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "张三",
                                  "studentNumber": "2024001",
                                  "college": "计算机学院",
                                  "major": "软件工程",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void unverifiedUserCanSubmitAndBecomesPendingReview() throws Exception {
        String email = "id-verify-submit@campus.edu.cn";
        String accessToken = registerAndLogin(email, "Str0ngPassword!", "IdVerify User");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "张三",
                                  "studentNumber": "2024001",
                                  "college": "计算机学院",
                                  "major": "软件工程",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("PENDING_REVIEW"))
                .andExpect(jsonPath("$.submittedAt", not(emptyOrNullString())))
                .andExpect(jsonPath("$.realName").value("张三"))
                .andExpect(jsonPath("$.studentNumber").value("2024001"));
    }

    @Test
    void queryOwnStatusAfterSubmission() throws Exception {
        String email = "id-verify-query@campus.edu.cn";
        String accessToken = registerAndLogin(email, "Str0ngPassword!", "IdQuery User");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "李四",
                                  "studentNumber": "2024002",
                                  "college": "数学学院",
                                  "major": "应用数学",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isOk());

        mockMvc.perform(get("/api/auth/identity-verifications/me")
                        .header("Authorization", "Bearer " + accessToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("PENDING_REVIEW"))
                .andExpect(jsonPath("$.reviewStatus").value("PENDING_REVIEW"))
                .andExpect(jsonPath("$.submittedAt", not(emptyOrNullString())))
                .andExpect(jsonPath("$.realName").value("李四"))
                .andExpect(jsonPath("$.studentNumber").value("2024002"))
                .andExpect(jsonPath("$.allowedActions").value(hasItem("RESUBMIT")));
    }

    @Test
    void submitWithMissingFieldsReturnsValidationFailed() throws Exception {
        String email = "id-verify-invalid@campus.edu.cn";
        String accessToken = registerAndLogin(email, "Str0ngPassword!", "IdInvalid User");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "",
                                  "studentNumber": "",
                                  "college": "计算机学院"
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void pendingReviewUserCannotResubmit() throws Exception {
        String email = "id-verify-dup@campus.edu.cn";
        String accessToken = registerAndLogin(email, "Str0ngPassword!", "IdDup User");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "王五",
                                  "studentNumber": "2024003",
                                  "college": "物理学院",
                                  "major": "物理学",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isOk());

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "王五2",
                                  "studentNumber": "2024003",
                                  "college": "物理学院",
                                  "major": "物理学",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("IDENTITY_VERIFICATION_PENDING"));
    }

    @Test
    void rejectedUserCanResubmit() throws Exception {
        String email = "id-verify-rejected@campus.edu.cn";
        String accessToken = registerAndLogin(email, "Str0ngPassword!", "IdReject User");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "赵六",
                                  "studentNumber": "2024004",
                                  "college": "化学学院",
                                  "major": "化学",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isOk());

        UUID userId = userAccountRepository.findByCampusEmail(email).orElseThrow().getUserId();
        IdentityVerificationSubmission sub = submissionRepository.findByUserId(userId).orElseThrow();
        sub.setReviewStatus("REJECTED");
        sub.setRejectReason("材料不清晰");
        sub.setReviewedAt(Instant.now());
        submissionRepository.save(sub);
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("REJECTED");
        userAccountRepository.save(account);

        String newAccessToken = login(email, "Str0ngPassword!");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + newAccessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "赵六",
                                  "studentNumber": "2024004",
                                  "college": "化学学院",
                                  "major": "化学",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("PENDING_REVIEW"));
    }

    @Test
    void verifiedUserCannotResubmit() throws Exception {
        String email = "id-verify-verified@campus.edu.cn";
        String accessToken = registerAndLogin(email, "Str0ngPassword!", "IdVerified User");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + accessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "孙七",
                                  "studentNumber": "2024005",
                                  "college": "文学院",
                                  "major": "中文",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isOk());

        UUID userId = userAccountRepository.findByCampusEmail(email).orElseThrow().getUserId();
        IdentityVerificationSubmission sub = submissionRepository.findByUserId(userId).orElseThrow();
        sub.setReviewStatus("APPROVED");
        sub.setReviewedAt(Instant.now());
        submissionRepository.save(sub);
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("VERIFIED");
        userAccountRepository.save(account);

        String newAccessToken = login(email, "Str0ngPassword!");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + newAccessToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "realName": "孙七",
                                  "studentNumber": "2024005",
                                  "college": "文学院",
                                  "major": "中文",
                                  "grade": "2024"
                                }
                                """))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("IDENTITY_ALREADY_VERIFIED"));
    }

    private String registerAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "verificationTicket": "%s",
                                  "password": "%s",
                                  "displayName": "%s"
                                }
                                """.formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        return login(email, password);
    }

    private String login(String email, String password) throws Exception {
        String response = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "password": "%s"
                                }
                                """.formatted(email, password)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return JsonPath.read(response, "$.accessToken");
    }

    private String verifiedTicket(String email) throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """.formatted(email)))
                .andExpect(status().isOk());

        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");
        String response = mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "code": "%s",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """.formatted(email, code)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return JsonPath.read(response, "$.verificationTicket");
    }

    @TestConfiguration
    static class TestConfig {

        @Bean
        @Primary
        CapturingCampusEmailVerificationCodeSender capturingSender() {
            return new CapturingCampusEmailVerificationCodeSender();
        }

        @Bean
        @Primary
        Clock testClock() {
            return Clock.fixed(Instant.parse("2026-05-18T00:00:00Z"), ZoneOffset.UTC);
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
