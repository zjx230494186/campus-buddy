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

import static org.hamcrest.Matchers.*;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class IdentityVerificationAdminReviewEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private CapturingCampusEmailVerificationCodeSender codeSender;

    @Autowired
    private UserAccountRepository userAccountRepository;

    @Autowired
    private IdentityVerificationSubmissionRepository submissionRepository;

    @Autowired
    private com.campusbuddy.security.JwtService jwtService;

    @Test
    void studentCannotAccessAdminList() throws Exception {
        String email = "admin-test-student@campus.edu.cn";
        String studentToken = registerAndLogin(email, "Str0ngPassword!", "Student");

        mockMvc.perform(get("/api/admin/identity-verifications")
                        .header("Authorization", "Bearer " + studentToken))
                .andExpect(status().isForbidden());
    }

    @Test
    void unauthenticatedCannotAccessAdminEndpoints() throws Exception {
        mockMvc.perform(get("/api/admin/identity-verifications"))
                .andExpect(status().isUnauthorized());

        mockMvc.perform(post("/api/admin/identity-verifications/00000000-0000-0000-0000-000000000000/reviews")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVED\"}"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void adminCanListPendingSubmissions() throws Exception {
        String studentEmail = "admin-list-student@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "ListStudent");
        submitIdentity(studentToken);

        String adminToken = createAdminAndLogin("admin-list@campus.edu.cn", "AdminPass123!", "AdminList");

        mockMvc.perform(get("/api/admin/identity-verifications?status=PENDING_REVIEW")
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$", hasSize(greaterThanOrEqualTo(1))))
                .andExpect(jsonPath("$[0].reviewStatus").value("PENDING_REVIEW"))
                .andExpect(jsonPath("$[0].realName", not(emptyOrNullString())));
    }

    @Test
    void adminApproveUpdatesBothSubmissionAndAccount() throws Exception {
        String studentEmail = "admin-approve-student@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "ApproveStudent");
        String submissionId = submitIdentity(studentToken);

        String adminToken = createAdminAndLogin("admin-approve@campus.edu.cn", "AdminPass123!", "AdminApprove");

        mockMvc.perform(post("/api/admin/identity-verifications/" + submissionId + "/reviews")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVED\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.reviewStatus").value("APPROVED"))
                .andExpect(jsonPath("$.authenticationStatus").value("VERIFIED"))
                .andExpect(jsonPath("$.reviewedAt", not(emptyOrNullString())));

        String newStudentToken = login(studentEmail, "Str0ngPassword!");
        mockMvc.perform(get("/api/auth/identity-verifications/me")
                        .header("Authorization", "Bearer " + newStudentToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("VERIFIED"))
                .andExpect(jsonPath("$.reviewStatus").value("APPROVED"));
    }

    @Test
    void adminRejectUpdatesBothSubmissionAndAccount() throws Exception {
        String studentEmail = "admin-reject-student@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "RejectStudent");
        String submissionId = submitIdentity(studentToken);

        String adminToken = createAdminAndLogin("admin-reject@campus.edu.cn", "AdminPass123!", "AdminReject");

        mockMvc.perform(post("/api/admin/identity-verifications/" + submissionId + "/reviews")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"REJECTED\",\"rejectReason\":\"照片不清晰\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.reviewStatus").value("REJECTED"))
                .andExpect(jsonPath("$.authenticationStatus").value("REJECTED"))
                .andExpect(jsonPath("$.rejectReason").value("照片不清晰"))
                .andExpect(jsonPath("$.reviewedAt", not(emptyOrNullString())));

        String newStudentToken = login(studentEmail, "Str0ngPassword!");
        mockMvc.perform(get("/api/auth/identity-verifications/me")
                        .header("Authorization", "Bearer " + newStudentToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("REJECTED"))
                .andExpect(jsonPath("$.rejectReason").value("照片不清晰"));
    }

    @Test
    void rejectWithoutReasonReturnsValidationFailed() throws Exception {
        String studentEmail = "admin-noreason-student@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "NoReasonStudent");
        String submissionId = submitIdentity(studentToken);

        String adminToken = createAdminAndLogin("admin-noreason@campus.edu.cn", "AdminPass123!", "AdminNoReason");

        mockMvc.perform(post("/api/admin/identity-verifications/" + submissionId + "/reviews")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"REJECTED\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void reviewNonPendingSubmissionReturnsConflict() throws Exception {
        String studentEmail = "admin-dupreview-student@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "DupReviewStudent");
        String submissionId = submitIdentity(studentToken);

        String adminToken = createAdminAndLogin("admin-dupreview@campus.edu.cn", "AdminPass123!", "AdminDupReview");

        mockMvc.perform(post("/api/admin/identity-verifications/" + submissionId + "/reviews")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"APPROVED\"}"))
                .andExpect(status().isOk());

        mockMvc.perform(post("/api/admin/identity-verifications/" + submissionId + "/reviews")
                        .header("Authorization", "Bearer " + adminToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"decision\":\"REJECTED\",\"rejectReason\":\"再次驳回\"}"))
                .andExpect(status().isConflict())
                .andExpect(jsonPath("$.code").value("SUBMISSION_NOT_PENDING"));
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

    private String createAdminAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {"campusEmail":"%s","verificationTicket":"%s","password":"%s","displayName":"%s"}
                                """.formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAccountRole("ADMIN");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String submitIdentity(String token) throws Exception {
        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("""
                                {"realName":"测试实名","studentNumber":"2024001","college":"计算机学院","major":"软件工程","grade":"2024"}
                                """))
                .andExpect(status().isOk());

        java.util.UUID userId = jwtService.getUserIdFromToken(token);
        var sub = submissionRepository.findByUserId(userId).orElseThrow();
        return sub.getSubmissionId().toString();
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

    @TestConfiguration
    static class TestConfig {
        @Bean @Primary
        CapturingCampusEmailVerificationCodeSender capturingSender() {
            return new CapturingCampusEmailVerificationCodeSender();
        }
        @Bean @Primary
        Clock testClock() {
            return Clock.fixed(Instant.parse("2026-05-19T00:00:00Z"), ZoneOffset.UTC);
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
