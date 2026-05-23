package com.campusbuddy.contact;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.CampusEmailVerificationCodeSender;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostRepository;
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
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class ContactConversationEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository partnerPostRepository;
    @Autowired private ConversationRepository conversationRepository;
    @Autowired private ConversationMessageRepository conversationMessageRepository;

    @BeforeEach
    void cleanUp() {
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();
    }

    @Test
    void contactRequestRequiresAuthentication() throws Exception {
        UUID postId = UUID.randomUUID();
        mockMvc.perform(post("/api/partner-posts/" + postId + "/contact-requests")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"hello\"}"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void unverifiedUserCannotRequestContact() throws Exception {
        String token = registerAndLogin("cc-unver@campus.edu.cn", "Str0ngPassword!", "CCUnver");
        UUID publisherId = getUserId("cc-unver@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Unver Test Post");
        UUID otherId = createVerifiedUser("cc-unver2@campus.edu.cn", "Str0ngPassword!", "CCUnver2");
        String otherToken = login("cc-unver2@campus.edu.cn", "Str0ngPassword!");

        UserAccount unverifiedAccount = userAccountRepository.findByCampusEmail("cc-unver2@campus.edu.cn").orElseThrow();
        unverifiedAccount.setAuthenticationStatus("UNVERIFIED");
        userAccountRepository.save(unverifiedAccount);

        mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + otherToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"hello\"}"))
                .andExpect(status().isForbidden());
    }

    @Test
    void publisherCannotContactOwnPost() throws Exception {
        String token = registerVerifiedAndLogin("cc-ownpub@campus.edu.cn", "Str0ngPassword!", "CCOwnPub");
        UUID publisherId = getUserId("cc-ownpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Own Post Test");

        mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"hello\"}"))
                .andExpect(status().isForbidden());
    }

    @Test
    void nonPublishedPostCannotBeContacted() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-draftpub@campus.edu.cn", "Str0ngPassword!", "CCDraftPub");
        UUID publisherId = getUserId("cc-draftpub@campus.edu.cn");
        PartnerPost draftPost = new PartnerPost(publisherId, "DRAFT", Instant.now());
        draftPost.setTitle("Draft Post");
        partnerPostRepository.save(draftPost);

        String requesterToken = registerVerifiedAndLogin("cc-draftreq@campus.edu.cn", "Str0ngPassword!", "CCDraftReq");

        mockMvc.perform(post("/api/partner-posts/" + draftPost.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"hello\"}"))
                .andExpect(status().isNotFound());
    }

    @Test
    void unverifiedPublisherCannotBeContacted() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-unvpub@campus.edu.cn", "Str0ngPassword!", "CCUnvPub");
        UUID publisherId = getUserId("cc-unvpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Unv Pub Post");

        UserAccount publisher = userAccountRepository.findByCampusEmail("cc-unvpub@campus.edu.cn").orElseThrow();
        publisher.setAuthenticationStatus("UNVERIFIED");
        userAccountRepository.save(publisher);

        String requesterToken = registerVerifiedAndLogin("cc-unvreq@campus.edu.cn", "Str0ngPassword!", "CCUnvReq");

        mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"hello\"}"))
                .andExpect(status().isForbidden());
    }

    @Test
    void verifiedUserCanRequestContact() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-okpub@campus.edu.cn", "Str0ngPassword!", "CCOkPub");
        UUID publisherId = getUserId("cc-okpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "OK Pub Post");

        String requesterToken = registerVerifiedAndLogin("cc-okreq@campus.edu.cn", "Str0ngPassword!", "CCOkReq");

        mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"hi lets study\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.conversationId").exists())
                .andExpect(jsonPath("$.status").value("ACTIVE"));
    }

    @Test
    void contactRequestReusesActiveConversation() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-reusepub@campus.edu.cn", "Str0ngPassword!", "CCReusePub");
        UUID publisherId = getUserId("cc-reusepub@campus.edu.cn");
        PartnerPost post1 = createPublishedPost(publisherId, "Reuse Post 1");
        PartnerPost post2 = createPublishedPost(publisherId, "Reuse Post 2");

        String requesterToken = registerVerifiedAndLogin("cc-reusereq@campus.edu.cn", "Str0ngPassword!", "CCReuseReq");

        String firstResponse = mockMvc.perform(post("/api/partner-posts/" + post1.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"first message\"}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        Long firstConvId = ((Number) com.jayway.jsonpath.JsonPath.read(firstResponse, "$.conversationId")).longValue();

        String secondResponse = mockMvc.perform(post("/api/partner-posts/" + post2.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"second message\"}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        Long secondConvId = ((Number) com.jayway.jsonpath.JsonPath.read(secondResponse, "$.conversationId")).longValue();

        org.junit.jupiter.api.Assertions.assertEquals(firstConvId, secondConvId,
                "Should reuse the same ACTIVE conversation for the same pair of users");
    }

    @Test
    void blankMessageReturnsValidationError() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-blankpub@campus.edu.cn", "Str0ngPassword!", "CCBlankPub");
        UUID publisherId = getUserId("cc-blankpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Blank Post");

        String requesterToken = registerVerifiedAndLogin("cc-blankreq@campus.edu.cn", "Str0ngPassword!", "CCBlankReq");

        mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"   \"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void tooLongMessageReturnsValidationError() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-longpub@campus.edu.cn", "Str0ngPassword!", "CCLongPub");
        UUID publisherId = getUserId("cc-longpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Long Post");

        String requesterToken = registerVerifiedAndLogin("cc-longreq@campus.edu.cn", "Str0ngPassword!", "CCLongReq");

        String longMsg = "a".repeat(31);
        mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"" + longMsg + "\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void participantCanSendMessage() throws Exception {
        Long convId = setupConversationWithMessage("cc-sendmsg");

        UUID requesterId = getUserId("cc-sendmsgreq@campus.edu.cn");
        String requesterToken = login("cc-sendmsgreq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/messages")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"a follow up\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.messageId").exists());
    }

    @Test
    void nonParticipantCannotSendMessage() throws Exception {
        Long convId = setupConversationWithMessage("cc-nonpar");

        String outsiderToken = registerVerifiedAndLogin("cc-outsider@campus.edu.cn", "Str0ngPassword!", "CCOutsider");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/messages")
                        .header("Authorization", "Bearer " + outsiderToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"intruder\"}"))
                .andExpect(status().isForbidden());
    }

    @Test
    void participantCanListConversations() throws Exception {
        Long convId = setupConversationWithMessage("cc-listconv");

        String requesterToken = login("cc-listconvreq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))))
                .andExpect(jsonPath("$.items[?(@.conversationId==" + convId + ")]").exists());
    }

    @Test
    void conversationListContainsLastMessagePreview() throws Exception {
        Long convId = setupConversationWithMessage("cc-preview");

        String requesterToken = login("cc-previewreq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items[?(@.conversationId==" + convId + ")].lastMessagePreview").exists());
    }

    @Test
    void conversationListExcludesSensitiveFields() throws Exception {
        setupConversationWithMessage("cc-nosens");

        String requesterToken = login("cc-nosensreq@campus.edu.cn", "Str0ngPassword!");

        String response = mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        for (String field : new String[]{"campusEmail", "studentNumber", "realName", "passwordHash"}) {
            org.junit.jupiter.api.Assertions.assertFalse(
                    response.contains("\"" + field + "\""),
                    "Conversation list must not contain: " + field
            );
        }
    }

    @Test
    void participantCanQueryMessages() throws Exception {
        Long convId = setupConversationWithMessage("cc-querymsg");

        String requesterToken = login("cc-querymsgreq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(get("/api/me/conversations/" + convId + "/messages")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))));
    }

    @Test
    void nonParticipantCannotQueryMessages() throws Exception {
        Long convId = setupConversationWithMessage("cc-nonparquery");

        String outsiderToken = registerVerifiedAndLogin("cc-outsider2@campus.edu.cn", "Str0ngPassword!", "CCOutsider2");

        mockMvc.perform(get("/api/me/conversations/" + convId + "/messages")
                        .header("Authorization", "Bearer " + outsiderToken))
                .andExpect(status().isForbidden());
    }

    @Test
    void isValidConversationStillWorksAfterNewFeatures() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cc-ctxpub@campus.edu.cn", "Str0ngPassword!", "CCCtxPub");
        UUID publisherId = getUserId("cc-ctxpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Ctx Post");

        String requesterToken = registerVerifiedAndLogin("cc-ctxreq@campus.edu.cn", "Str0ngPassword!", "CCCtxReq");
        UUID requesterId = getUserId("cc-ctxreq@campus.edu.cn");

        String response = mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"init msg\"}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        Long convId = ((Number) com.jayway.jsonpath.JsonPath.read(response, "$.conversationId")).longValue();

        Instant now = Instant.now();
        conversationMessageRepository.save(new ConversationMessage(convId, publisherId, "USER_TEXT", "reply 1", now));
        conversationMessageRepository.save(new ConversationMessage(convId, publisherId, "USER_TEXT", "reply 1b", now.plusSeconds(1)));
        conversationMessageRepository.save(new ConversationMessage(convId, requesterId, "USER_TEXT", "reply 2", now.plusSeconds(2)));

        ContactContextService contactContextService = applicationContext.getBean(ContactContextService.class);
        org.junit.jupiter.api.Assertions.assertTrue(contactContextService.isValidConversation(convId));
    }

    @Autowired private org.springframework.context.ApplicationContext applicationContext;

    private PartnerPost createPublishedPost(UUID publisherId, String title) {
        PartnerPost post = new PartnerPost(publisherId, "PUBLISHED", Instant.now());
        post.setTitle(title);
        post.setSceneType("STUDY");
        post.setPublishedAt(Instant.now());
        return partnerPostRepository.save(post);
    }

    private UUID getUserId(String email) {
        return userAccountRepository.findByCampusEmail(email).map(UserAccount::getUserId).orElseThrow();
    }

    private Long setupConversationWithMessage(String prefix) throws Exception {
        String publisherToken = registerVerifiedAndLogin(prefix + "pub@campus.edu.cn", "Str0ngPassword!", prefix + "Pub");
        UUID publisherId = getUserId(prefix + "pub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, prefix + " Post");

        String requesterToken = registerVerifiedAndLogin(prefix + "req@campus.edu.cn", "Str0ngPassword!", prefix + "Req");

        String response = mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"initial invite\"}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return ((Number) com.jayway.jsonpath.JsonPath.read(response, "$.conversationId")).longValue();
    }

    private UUID createVerifiedUser(String email, String password, String displayName) throws Exception {
        registerVerifiedAndLogin(email, password, displayName);
        return getUserId(email);
    }

    private String registerVerifiedAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("VERIFIED");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String registerAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        return login(email, password);
    }

    private String login(String email, String password) throws Exception {
        String response = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"password\":\"%s\"}".formatted(email, password)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.accessToken");
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
        return com.jayway.jsonpath.JsonPath.read(response, "$.verificationTicket");
    }

    @TestConfiguration
    static class TestConfig {
        @Bean @Primary CapturingCampusEmailVerificationCodeSender capturingSender() { return new CapturingCampusEmailVerificationCodeSender(); }
        @Bean @Primary Clock testClock() { return Clock.fixed(Instant.parse("2026-05-23T00:00:00Z"), ZoneOffset.UTC); }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();
        @Override public void send(String campusEmail, String verificationCode, String purpose) { codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode); }
        String latestCode(String campusEmail, String purpose) { return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase()); }
    }
}
