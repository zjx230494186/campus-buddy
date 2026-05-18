package com.campusbuddy.auth;

import com.campusbuddy.TestcontainersConfiguration;
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
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class AuthPersistenceIntegrationTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private CapturingCampusEmailVerificationCodeSender codeSender;

    @Test
    void fullRegistrationAndLoginFlowWithRealJwt() throws Exception {
        String email = "integration@campus.edu.cn";

        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"" + email + "\",\"purpose\":\"REGISTER_OR_LOGIN\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.verificationStatus").value("CODE_SENT"));

        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");

        String verificationResponse = mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"" + email + "\",\"code\":\"" + code + "\",\"purpose\":\"REGISTER_OR_LOGIN\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.verificationStatus").value("VERIFIED"))
                .andExpect(jsonPath("$.verificationTicket", not(emptyOrNullString())))
                .andReturn().getResponse().getContentAsString();

        String ticket = new com.fasterxml.jackson.databind.ObjectMapper()
                .readTree(verificationResponse).get("verificationTicket").asText();

        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"" + email + "\",\"verificationTicket\":\"" + ticket + "\",\"password\":\"TestPass123\",\"displayName\":\"IntegrationUser\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("UNVERIFIED"))
                .andExpect(jsonPath("$.campusEmailVerificationStatus").value("VERIFIED"));

        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"" + email + "\",\"verificationTicket\":\"" + ticket + "\",\"password\":\"TestPass123\",\"displayName\":\"DupUser\"}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("EMAIL_ALREADY_REGISTERED"));

        String loginResponse = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"" + email + "\",\"password\":\"TestPass123\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.accessToken", not(emptyOrNullString())))
                .andExpect(jsonPath("$.refreshToken", not(emptyOrNullString())))
                .andExpect(jsonPath("$.tokenType").value("Bearer"))
                .andExpect(jsonPath("$.accessTokenExpiresInSeconds").value(900))
                .andExpect(jsonPath("$.user.displayName").value("IntegrationUser"))
                .andReturn().getResponse().getContentAsString();

        String accessToken = new com.fasterxml.jackson.databind.ObjectMapper()
                .readTree(loginResponse).get("accessToken").asText();

        mockMvc.perform(get("/api/probe/secure")
                        .header("Authorization", "Bearer " + accessToken))
                .andExpect(status().isOk());

        mockMvc.perform(get("/api/probe/secure"))
                .andExpect(status().isUnauthorized());

        mockMvc.perform(get("/api/probe/secure")
                        .header("Authorization", "Bearer invalid-jwt-token"))
                .andExpect(status().isUnauthorized());

        mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"" + email + "\",\"password\":\"WrongPass123\"}"))
                .andExpect(status().isUnauthorized())
                .andExpect(jsonPath("$.code").value("INVALID_LOGIN_CREDENTIALS"));
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
