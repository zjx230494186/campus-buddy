package com.campusbuddy.auth;

import com.campusbuddy.TestcontainersConfiguration;
import com.jayway.jsonpath.JsonPath;
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

import static org.hamcrest.Matchers.emptyOrNullString;
import static org.hamcrest.Matchers.not;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class AuthLoginEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private CapturingCampusEmailVerificationCodeSender codeSender;

    @Test
    void loginReturnsPlaceholderTokensForRegisteredAccount() throws Exception {
        String email = "login-success@campus.edu.cn";
        String password = "Str0ngPassword!";
        register(email, password, "Login Tester");

        mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "password": "%s",
                                  "clientName": "windows-qt",
                                  "deviceId": "test-device"
                                }
                                """.formatted(email, password)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.accessToken", not(emptyOrNullString())))
                .andExpect(jsonPath("$.accessTokenExpiresInSeconds").value(900))
                .andExpect(jsonPath("$.refreshToken", not(emptyOrNullString())))
                .andExpect(jsonPath("$.refreshTokenExpiresInSeconds").value(2592000))
                .andExpect(jsonPath("$.tokenType").value("Bearer"))
                .andExpect(jsonPath("$.user.userId", not(emptyOrNullString())))
                .andExpect(jsonPath("$.user.displayName").value("Login Tester"))
                .andExpect(jsonPath("$.user.authenticationStatus").value("UNVERIFIED"))
                .andExpect(jsonPath("$.user.campusEmailVerificationStatus").value("VERIFIED"));
    }

    @Test
    void loginRejectsInvalidCredentials() throws Exception {
        String email = "login-invalid-password@campus.edu.cn";
        register(email, "Str0ngPassword!", "Invalid Password Tester");

        mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "password": "WrongPassword!"
                                }
                                """.formatted(email)))
                .andExpect(status().isUnauthorized())
                .andExpect(jsonPath("$.code").value("INVALID_LOGIN_CREDENTIALS"))
                .andExpect(jsonPath("$.message").value("Invalid login credentials"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void loginRejectsMissingOrMalformedFields() throws Exception {
        mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "not-an-email",
                                  "password": ""
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.message").value("Validation failed"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    private void register(String email, String password, String displayName) throws Exception {
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
                .andReturn()
                .getResponse()
                .getContentAsString();
        return JsonPath.read(response, "$.verificationTicket");
    }

    @TestConfiguration
    static class LoginTestConfig {

        @Bean
        @Primary
        CapturingCampusEmailVerificationCodeSender capturingCampusEmailVerificationCodeSender() {
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
            codes.put(key(campusEmail, purpose), verificationCode);
        }

        String latestCode(String campusEmail, String purpose) {
            return codes.get(key(campusEmail, purpose));
        }

        private String key(String campusEmail, String purpose) {
            return campusEmail.toLowerCase() + "|" + purpose.toUpperCase();
        }
    }
}
