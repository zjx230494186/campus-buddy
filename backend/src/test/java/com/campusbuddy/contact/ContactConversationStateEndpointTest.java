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
class ContactConversationStateEndpointTest {

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
    void participantCanCloseConversation() throws Exception {
        Long convId = setupConversation("cs-close");
        String requesterToken = login("cs-closereq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.conversationId").value(convId))
                .andExpect(jsonPath("$.status").value("CLOSED"));
    }

    @Test
    void nonParticipantCannotCloseConversation() throws Exception {
        Long convId = setupConversation("cs-nonparclose");
        String outsiderToken = registerVerifiedAndLogin("cs-outsider@campus.edu.cn", "Str0ngPassword!", "CSOutsider");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + outsiderToken))
                .andExpect(status().isForbidden());
    }

    @Test
    void closeConversationIsIdempotent() throws Exception {
        Long convId = setupConversation("cs-idemclose");
        String requesterToken = login("cs-idemclosereq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("CLOSED"));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("CLOSED"));
    }

    @Test
    void closedConversationRejectsMessage() throws Exception {
        Long convId = setupConversation("cs-closedmsg");
        String requesterToken = login("cs-closedmsgreq@campus.edu.cn", "Str0ngPassword!");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());

        mockMvc.perform(post("/api/me/conversations/" + convId + "/messages")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"should fail\"}"))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("CONVERSATION_CLOSED"));
    }

    @Test
    void recontactReopensClosedConversation() throws Exception {
        String publisherToken = registerVerifiedAndLogin("cs-reopenpub@campus.edu.cn", "Str0ngPassword!", "CSReopenPub");
        UUID publisherId = getUserId("cs-reopenpub@campus.edu.cn");
        PartnerPost post = createPublishedPost(publisherId, "Reopen Post");

        String requesterToken = registerVerifiedAndLogin("cs-reopenreq@campus.edu.cn", "Str0ngPassword!", "CSReopenReq");

        String firstResponse = mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"first contact\"}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        Long convId = ((Number) com.jayway.jsonpath.JsonPath.read(firstResponse, "$.conversationId")).longValue();

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());

        String secondResponse = mockMvc.perform(post("/api/partner-posts/" + post.getId() + "/contact-requests")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"message\":\"recontact\"}"))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        Long reopenedConvId = ((Number) com.jayway.jsonpath.JsonPath.read(secondResponse, "$.conversationId")).longValue();
        String reopenedStatus = com.jayway.jsonpath.JsonPath.read(secondResponse, "$.status");

        org.junit.jupiter.api.Assertions.assertEquals(convId, reopenedConvId, "Should reuse the same conversation");
        org.junit.jupiter.api.Assertions.assertEquals("ACTIVE", reopenedStatus, "Status should be ACTIVE after reopening");
    }

    @Test
    void conversationListIncludesUnreadCount() throws Exception {
        Long convId = setupConversation("cs-unread");
        UUID publisherId = getUserId("cs-unreadpub@campus.edu.cn");
        String requesterToken = login("cs-unreadreq@campus.edu.cn", "Str0ngPassword!");

        conversationMessageRepository.save(
                new ConversationMessage(convId, publisherId, "USER_TEXT", "new msg 1", Instant.now()));
        conversationMessageRepository.save(
                new ConversationMessage(convId, publisherId, "USER_TEXT", "new msg 2", Instant.now().plusSeconds(1)));

        String response = mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        int unreadCount = readUnreadCount(response, convId);
        org.junit.jupiter.api.Assertions.assertTrue(unreadCount >= 2,
                "Expected unreadCount >= 2 but was " + unreadCount);
    }

    @Test
    void ownMessagesDoNotCountAsUnread() throws Exception {
        Long convId = setupConversation("cs-ownunread");
        UUID requesterId = getUserId("cs-ownunreadreq@campus.edu.cn");
        String requesterToken = login("cs-ownunreadreq@campus.edu.cn", "Str0ngPassword!");

        conversationMessageRepository.save(
                new ConversationMessage(convId, requesterId, "USER_TEXT", "my own msg", Instant.now()));

        String response = mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        int unreadCount = readUnreadCount(response, convId);
        org.junit.jupiter.api.Assertions.assertEquals(0, unreadCount);
    }

    @Test
    void markReadClearsUnreadCount() throws Exception {
        Long convId = setupConversation("cs-markread");
        UUID publisherId = getUserId("cs-markreadpub@campus.edu.cn");
        String requesterToken = login("cs-markreadreq@campus.edu.cn", "Str0ngPassword!");

        conversationMessageRepository.save(
                new ConversationMessage(convId, publisherId, "USER_TEXT", "unread msg", Instant.now()));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/read")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());

        String response = mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        int unreadCount = readUnreadCount(response, convId);
        org.junit.jupiter.api.Assertions.assertEquals(0, unreadCount);
    }

    @Test
    void markReadDoesNotAffectOtherParticipantUnread() throws Exception {
        Long convId = setupConversation("cs-otherunread");
        UUID requesterId = getUserId("cs-otherunreadreq@campus.edu.cn");
        UUID publisherId = getUserId("cs-otherunreadpub@campus.edu.cn");
        String publisherToken = login("cs-otherunreadpub@campus.edu.cn", "Str0ngPassword!");

        conversationMessageRepository.save(
                new ConversationMessage(convId, requesterId, "USER_TEXT", "msg to publisher", Instant.now()));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/read")
                        .header("Authorization", "Bearer " + publisherToken))
                .andExpect(status().isOk());

        String pubResponse = mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + publisherToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        int pubUnread = readUnreadCount(pubResponse, convId);
        org.junit.jupiter.api.Assertions.assertEquals(0, pubUnread);

        String requesterToken = login("cs-otherunreadreq@campus.edu.cn", "Str0ngPassword!");
        String reqResponse = mockMvc.perform(get("/api/me/conversations")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        int reqUnread = readUnreadCount(reqResponse, convId);
        org.junit.jupiter.api.Assertions.assertEquals(0, reqUnread);
    }

    @Test
    void markReadIsIdempotent() throws Exception {
        Long convId = setupConversation("cs-idemread");
        UUID publisherId = getUserId("cs-idemreadpub@campus.edu.cn");
        String requesterToken = login("cs-idemreadreq@campus.edu.cn", "Str0ngPassword!");

        conversationMessageRepository.save(
                new ConversationMessage(convId, publisherId, "USER_TEXT", "msg", Instant.now()));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/read")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());

        mockMvc.perform(post("/api/me/conversations/" + convId + "/read")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());
    }

    @Test
    void nonParticipantCannotMarkRead() throws Exception {
        Long convId = setupConversation("cs-nonparread");
        String outsiderToken = registerVerifiedAndLogin("cs-readoutsider@campus.edu.cn", "Str0ngPassword!", "CSReadOutsider");

        mockMvc.perform(post("/api/me/conversations/" + convId + "/read")
                        .header("Authorization", "Bearer " + outsiderToken))
                .andExpect(status().isForbidden());
    }

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

    private Long setupConversation(String prefix) throws Exception {
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

    private int readUnreadCount(String response, Long convId) {
        net.minidev.json.JSONArray items = com.jayway.jsonpath.JsonPath.parse(response).read("$.items");
        for (Object item : items) {
            if (item instanceof java.util.Map<?, ?> m) {
                Object cid = m.get("conversationId");
                if (cid instanceof Number n && n.longValue() == convId.longValue()) {
                    Object uc = m.get("unreadCount");
                    if (uc instanceof Number un) return un.intValue();
                }
            }
        }
        throw new AssertionError("Cannot read unreadCount for conversationId=" + convId + " in response");
    }

    @TestConfiguration
    static class TestConfig {
        @Bean @Primary CapturingCampusEmailVerificationCodeSender capturingSender() { return new CapturingCampusEmailVerificationCodeSender(); }
        @Bean @Primary Clock testClock() { return Clock.fixed(Instant.parse("2026-05-24T00:00:00Z"), ZoneOffset.UTC); }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();
        @Override public void send(String campusEmail, String verificationCode, String purpose) { codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode); }
        String latestCode(String campusEmail, String purpose) { return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase()); }
    }
}
