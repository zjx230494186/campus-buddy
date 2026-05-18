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

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class AuthRegistrationEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private CapturingCampusEmailVerificationCodeSender codeSender;

    @Autowired
    private AuthRegistrationService registrationService;

    @Autowired
    private UserAccountRepository userAccountRepository;

    @Test
    void registerCreatesUnverifiedAccountForVerifiedCampusEmail() throws Exception {
        String email = "register-success@campus.edu.cn";
        String ticket = verifiedTicket(email);

        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "verificationTicket": "%s",
                                  "password": "Str0ngPassword!",
                                  "displayName": "Registration Tester"
                                }
                                """.formatted(email, ticket)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.userId", not(emptyOrNullString())))
                .andExpect(jsonPath("$.campusEmail").value(startsWith("re***@campus.edu.cn")))
                .andExpect(jsonPath("$.displayName").value("Registration Tester"))
                .andExpect(jsonPath("$.authenticationStatus").value("UNVERIFIED"))
                .andExpect(jsonPath("$.campusEmailVerificationStatus").value("VERIFIED"))
                .andExpect(jsonPath("$.createdAt", not(emptyOrNullString())));
    }

    @Test
    void registerStoresBCryptPasswordHashOnly() throws Exception {
        String email = "register-password-hash@campus.edu.cn";
        String rawPassword = "Str0ngPassword!";
        String ticket = verifiedTicket(email);

        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "verificationTicket": "%s",
                                  "password": "%s",
                                  "displayName": "Password Hash Tester"
                                }
                                """.formatted(email, ticket, rawPassword)))
                .andExpect(status().isOk());

        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        org.assertj.core.api.Assertions.assertThat(account.getPasswordHash())
                .isNotEqualTo(rawPassword)
                .startsWith("$2");
    }


    @Test
    void registerRejectsInvalidVerificationTicket() throws Exception {
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "register-unverified@campus.edu.cn",
                                  "verificationTicket": "cet_invalid",
                                  "password": "Str0ngPassword!",
                                  "displayName": "Unverified User"
                                }
                                """))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("CAMPUS_EMAIL_NOT_VERIFIED"))
                .andExpect(jsonPath("$.message").value("Campus email not verified"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void registerRejectsDuplicateCampusEmail() throws Exception {
        String email = "register-duplicate@campus.edu.cn";
        String firstTicket = verifiedTicket(email);
        register(email, firstTicket, "First User");

        String secondTicket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "verificationTicket": "%s",
                                  "password": "Str0ngPassword!",
                                  "displayName": "Second User"
                                }
                                """.formatted(email, secondTicket)))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("EMAIL_ALREADY_REGISTERED"))
                .andExpect(jsonPath("$.message").value("Email already registered"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void registerRejectsInvalidCampusEmailDomain() throws Exception {
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "student@example.com",
                                  "verificationTicket": "cet_invalid",
                                  "password": "Str0ngPassword!",
                                  "displayName": "Invalid Domain"
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("INVALID_CAMPUS_EMAIL_DOMAIN"))
                .andExpect(jsonPath("$.message").value("Invalid campus email domain"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void registerRejectsMissingOrMalformedFields() throws Exception {
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "not-an-email",
                                  "verificationTicket": "",
                                  "password": "short",
                                  "displayName": ""
                                }
                                """))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"))
                .andExpect(jsonPath("$.message").value("Validation failed"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    private void register(String email, String ticket, String displayName) throws Exception {
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {
                                  "campusEmail": "%s",
                                  "verificationTicket": "%s",
                                  "password": "Str0ngPassword!",
                                  "displayName": "%s"
                                }
                                """.formatted(email, ticket, displayName)))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.campusEmail").value(startsWith(email.substring(0, 2) + "***@campus.edu.cn")));
    }

    private String verifiedTicket(String email) throws Exception {
        sendCode(email);
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
                .andExpect(jsonPath("$.verificationStatus").value("VERIFIED"))
                .andReturn()
                .getResponse()
                .getContentAsString();
        return JsonPath.read(response, "$.verificationTicket");
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
    static class RegistrationTestConfig {

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
