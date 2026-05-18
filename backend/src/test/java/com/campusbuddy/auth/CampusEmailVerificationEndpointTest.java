package com.campusbuddy.auth;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Primary;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneId;
import java.time.ZoneOffset;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import static org.hamcrest.Matchers.emptyOrNullString;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.startsWith;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@SpringBootTest(properties = "spring.autoconfigure.exclude=org.springframework.boot.jdbc.autoconfigure.DataSourceAutoConfiguration")
@AutoConfigureMockMvc
class CampusEmailVerificationEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private CapturingCampusEmailVerificationCodeSender codeSender;

    @Autowired
    private MutableTestClock testClock;

    @Test
    void sendCodeReturnsCodeSentForAllowedCampusEmail() throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "student@campus.edu.cn",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.campusEmail").value(startsWith("st***@campus.edu.cn")))
                .andExpect(jsonPath("$.verificationStatus").value("CODE_SENT"))
                .andExpect(jsonPath("$.expiresInSeconds").value(600))
                .andExpect(jsonPath("$.resendAfterSeconds").value(60));
    }

    @Test
    void sendCodeRejectsInvalidCampusEmailDomain() throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "student@example.com",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("INVALID_CAMPUS_EMAIL_DOMAIN"))
                .andExpect(jsonPath("$.message").value("Invalid campus email domain"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void sendCodeRejectsMissingOrMalformedFields() throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "not-an-email"
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.message").value("Validation failed"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void sendCodeRejectsTooFrequentRequests() throws Exception {
        String requestBody = """
                {
                  "campusEmail": "cooldown@campus.edu.cn",
                  "purpose": "REGISTER_OR_LOGIN"
                }
                """;

        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(requestBody))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.verificationStatus").value("CODE_SENT"));

        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content(requestBody))
                .andExpect(status().isTooManyRequests())
                .andExpect(jsonPath("$.code").value("EMAIL_VERIFICATION_TOO_FREQUENT"))
                .andExpect(jsonPath("$.message").value("Email verification request too frequent"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void verifyCodeReturnsVerifiedForCorrectCode() throws Exception {
        String email = "verify-success@campus.edu.cn";
        sendCode(email);
        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");

        mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "code": "%s",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """.formatted(email, code)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.campusEmail").value(startsWith("ve***@campus.edu.cn")))
                .andExpect(jsonPath("$.verificationStatus").value("VERIFIED"))
                .andExpect(jsonPath("$.verifiedAt", not(emptyOrNullString())))
                .andExpect(jsonPath("$.verificationTicket", not(emptyOrNullString())));
    }

    @Test
    void verifyCodeRejectsInvalidCode() throws Exception {
        String email = "verify-invalid@campus.edu.cn";
        sendCode(email);
        String invalidCode = differentCode(codeSender.latestCode(email, "REGISTER_OR_LOGIN"));

        mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "code": "%s",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """.formatted(email, invalidCode)))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("EMAIL_VERIFICATION_CODE_INVALID"))
                .andExpect(jsonPath("$.message").value("Email verification code invalid"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void verifyCodeRejectsExpiredCode() throws Exception {
        String email = "verify-expired@campus.edu.cn";
        sendCode(email);
        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");
        testClock.advanceSeconds(CampusEmailVerificationService.CODE_EXPIRES_IN_SECONDS + 1L);

        mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "code": "%s",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """.formatted(email, code)))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("EMAIL_VERIFICATION_CODE_EXPIRED"))
                .andExpect(jsonPath("$.message").value("Email verification code expired"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void verifyCodeRejectsInvalidCampusEmailDomain() throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "student@example.com",
                                  "code": "123456",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("INVALID_CAMPUS_EMAIL_DOMAIN"))
                .andExpect(jsonPath("$.message").value("Invalid campus email domain"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void verifyCodeRejectsMissingOrMalformedFields() throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "not-an-email",
                                  "code": "",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.message").value("Validation failed"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    private String differentCode(String code) {
        return code.startsWith("0") ? "1" + code.substring(1) : "0" + code.substring(1);
    }

    private void sendCode(String email) throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "purpose": "REGISTER_OR_LOGIN"
                                }
                                """.formatted(email)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.verificationStatus").value("CODE_SENT"));
    }

    @TestConfiguration
    static class VerificationTestConfig {

        @Bean
        @Primary
        CapturingCampusEmailVerificationCodeSender capturingCampusEmailVerificationCodeSender() {
            return new CapturingCampusEmailVerificationCodeSender();
        }

        @Bean
        @Primary
        Clock testClock(MutableTestClock mutableTestClock) {
            return mutableTestClock;
        }

        @Bean
        MutableTestClock mutableTestClock() {
            return new MutableTestClock(Instant.parse("2026-05-18T00:00:00Z"));
        }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();

        @Override
        public void send(String campusEmail, String verificationCode, String purpose) {
            codes.put(key(campusEmail, purpose), verificationCode);
        }

        String latestCode(String campusEmail, String purpose) {
            return codes.get(key(campusEmail, purpose));
        }

        private String key(String campusEmail, String purpose) {
            return campusEmail.toLowerCase() + "|" + purpose.toUpperCase();
        }
    }

    static class MutableTestClock extends Clock {
        private Instant currentInstant;

        MutableTestClock(Instant currentInstant) {
            this.currentInstant = currentInstant;
        }

        void advanceSeconds(long seconds) {
            currentInstant = currentInstant.plusSeconds(seconds);
        }

        @Override
        public ZoneId getZone() {
            return ZoneOffset.UTC;
        }

        @Override
        public Clock withZone(ZoneId zone) {
            return this;
        }

        @Override
        public Instant instant() {
            return currentInstant;
        }
    }
}
