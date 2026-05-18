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
import org.springframework.mock.web.MockMultipartFile;
import org.springframework.test.web.servlet.MockMvc;

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneOffset;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;

import static org.hamcrest.Matchers.*;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.multipart;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class IdentityVerificationMaterialAttachmentEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private com.campusbuddy.security.JwtService jwtService;

    @Test
    void uploadRequiresAuthentication() throws Exception {
        MockMultipartFile file = new MockMultipartFile("file", "test.png", "image/png", new byte[]{1, 2, 3});
        mockMvc.perform(multipart("/api/auth/identity-verifications/materials").file(file))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void uploadValidPngReturnsAttachmentMetadata() throws Exception {
        String email = "mat-upload@campus.edu.cn";
        String token = registerAndLogin(email, "Str0ngPassword!", "MatUpload");

        byte[] pngData = new byte[]{(byte)0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A, 0x01, 0x02};
        MockMultipartFile file = new MockMultipartFile("file", "student_card.png", "image/png", pngData);

        mockMvc.perform(multipart("/api/auth/identity-verifications/materials")
                        .file(file)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.attachmentId", not(emptyOrNullString())))
                .andExpect(jsonPath("$.contentType").value("image/png"))
                .andExpect(jsonPath("$.sizeBytes").value(pngData.length))
                .andExpect(jsonPath("$.sha256", not(emptyOrNullString())))
                .andExpect(jsonPath("$.status").value("ACTIVE"));
    }

    @Test
    void uploadExceeding10MbReturnsTooLarge() throws Exception {
        String email = "mat-large@campus.edu.cn";
        String token = registerAndLogin(email, "Str0ngPassword!", "MatLarge");

        byte[] bigData = new byte[10 * 1024 * 1024 + 1];
        MockMultipartFile file = new MockMultipartFile("file", "huge.pdf", "application/pdf", bigData);

        mockMvc.perform(multipart("/api/auth/identity-verifications/materials")
                        .file(file)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("ATTACHMENT_TOO_LARGE"));
    }

    @Test
    void uploadDisallowedMimeReturnsTypeNotAllowed() throws Exception {
        String email = "mat-mime@campus.edu.cn";
        String token = registerAndLogin(email, "Str0ngPassword!", "MatMime");

        MockMultipartFile file = new MockMultipartFile("file", "malware.exe", "application/x-msdownload", new byte[]{1});

        mockMvc.perform(multipart("/api/auth/identity-verifications/materials")
                        .file(file)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("ATTACHMENT_TYPE_NOT_ALLOWED"));
    }

    @Test
    void submitWithOtherUsersAttachmentReturnsForbidden() throws Exception {
        String ownerEmail = "mat-owner@campus.edu.cn";
        String ownerToken = registerAndLogin(ownerEmail, "Str0ngPassword!", "MatOwner");
        String attachmentId = uploadMaterial(ownerToken, "image/png", "a.png", new byte[]{1, 2, 3});

        String otherEmail = "mat-other@campus.edu.cn";
        String otherToken = registerAndLogin(otherEmail, "Str0ngPassword!", "MatOther");

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + otherToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"realName\":\"Other\",\"studentNumber\":\"2024999\",\"college\":\"CS\",\"major\":\"SE\",\"grade\":\"2024\",\"materialAttachmentId\":\"" + attachmentId + "\"}"))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("ATTACHMENT_NOT_OWNED"));
    }

    @Test
    void submitWithOwnAttachmentSucceeds() throws Exception {
        String email = "mat-submit@campus.edu.cn";
        String token = registerAndLogin(email, "Str0ngPassword!", "MatSubmit");
        String attachmentId = uploadMaterial(token, "image/jpeg", "id.jpg", new byte[]{(byte)0xFF, (byte)0xD8, (byte)0xFF});

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"realName\":\"张三\",\"studentNumber\":\"2024001\",\"college\":\"CS\",\"major\":\"SE\",\"grade\":\"2024\",\"materialAttachmentId\":\"" + attachmentId + "\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticationStatus").value("PENDING_REVIEW"));
    }

    @Test
    void adminListIncludesAttachmentSummaryWithoutObjectKey() throws Exception {
        String studentEmail = "mat-adminlist@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "MatAdminList");
        String attachmentId = uploadMaterial(studentToken, "image/png", "card.png", new byte[]{1, 2, 3, 4});

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + studentToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"realName\":\"李四\",\"studentNumber\":\"2024002\",\"college\":\"Math\",\"major\":\"Applied\",\"grade\":\"2024\",\"materialAttachmentId\":\"" + attachmentId + "\"}"))
                .andExpect(status().isOk());

        String adminToken = createAdminAndLogin("mat-admin@campus.edu.cn", "AdminPass123!", "MatAdmin");

        mockMvc.perform(get("/api/admin/identity-verifications?status=PENDING_REVIEW")
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$[?(@.materialAttachmentId != null)].materialAttachmentId", hasSize(greaterThanOrEqualTo(1))))
                .andExpect(jsonPath("$[?(@.materialAttachmentId != null)].materialContentType", hasItem("image/png")));
    }

    @Test
    void adminCanReadMaterialContent() throws Exception {
        String studentEmail = "mat-adminread@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "MatAdminRead");
        String attachmentId = uploadMaterial(studentToken, "image/png", "read.png", new byte[]{(byte)0x89, (byte)0x50, (byte)0x4E, (byte)0x47});

        mockMvc.perform(post("/api/auth/identity-verifications")
                        .header("Authorization", "Bearer " + studentToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"realName\":\"王五\",\"studentNumber\":\"2024003\",\"college\":\"Phy\",\"major\":\"Phy\",\"grade\":\"2024\",\"materialAttachmentId\":\"" + attachmentId + "\"}"))
                .andExpect(status().isOk());

        String adminToken = createAdminAndLogin("mat-readadmin@campus.edu.cn", "AdminPass123!", "MatReadAdmin");

        java.util.UUID userId = jwtService.getUserIdFromToken(studentToken);
        var sub = new java.util.concurrent.ConcurrentHashMap<Class<?>, Object>();

        String listResponse = mockMvc.perform(get("/api/admin/identity-verifications?status=PENDING_REVIEW")
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        var submissionIds = JsonPath.read(listResponse, "$[?(@.materialAttachmentId == '" + attachmentId + "')].submissionId");
        String submissionId = (String) ((java.util.List<?>) submissionIds).get(0);

        mockMvc.perform(get("/api/admin/identity-verifications/" + submissionId + "/material")
                        .header("Authorization", "Bearer " + adminToken))
                .andExpect(status().isOk())
                .andExpect(header().string("Content-Type", "image/png"));
    }

    @Test
    void studentCannotReadAdminMaterialEndpoint() throws Exception {
        String studentEmail = "mat-studentread@campus.edu.cn";
        String studentToken = registerAndLogin(studentEmail, "Str0ngPassword!", "MatStudentRead");

        mockMvc.perform(get("/api/admin/identity-verifications/00000000-0000-0000-0000-000000000000/material")
                        .header("Authorization", "Bearer " + studentToken))
                .andExpect(status().isForbidden());
    }

    private String registerAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        return login(email, password);
    }

    private String createAdminAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAccountRole("ADMIN");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String uploadMaterial(String token, String contentType, String filename, byte[] data) throws Exception {
        MockMultipartFile file = new MockMultipartFile("file", filename, contentType, data);
        String response = mockMvc.perform(multipart("/api/auth/identity-verifications/materials")
                        .file(file)
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return JsonPath.read(response, "$.attachmentId");
    }

    private String login(String email, String password) throws Exception {
        String response = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"password\":\"%s\"}".formatted(email, password)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return JsonPath.read(response, "$.accessToken");
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
        return JsonPath.read(response, "$.verificationTicket");
    }

    @TestConfiguration
    static class TestConfig {
        @Bean @Primary CapturingCampusEmailVerificationCodeSender capturingSender() { return new CapturingCampusEmailVerificationCodeSender(); }
        @Bean @Primary Clock testClock() { return Clock.fixed(Instant.parse("2026-05-19T00:00:00Z"), ZoneOffset.UTC); }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();
        @Override public void send(String campusEmail, String verificationCode, String purpose) { codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode); }
        String latestCode(String campusEmail, String purpose) { return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase()); }
    }
}
