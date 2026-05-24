package com.campusbuddy.contact;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.CampusEmailVerificationCodeSender;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostRepository;
import com.campusbuddy.review.ReviewRepository;
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
class ContactCardUnlockEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository partnerPostRepository;
    @Autowired private ConversationRepository conversationRepository;
    @Autowired private ConversationMessageRepository conversationMessageRepository;
    @Autowired private ContactCardRepository contactCardRepository;
    @Autowired private ContactUnlockConfirmRepository contactUnlockConfirmRepository;
    @Autowired private ContactUnlockRecordRepository contactUnlockRecordRepository;
    @Autowired private ReviewRepository reviewRepository;

    @BeforeEach
    void cleanUp() {
        reviewRepository.deleteAll();
        contactUnlockRecordRepository.deleteAll();
        contactUnlockConfirmRepository.deleteAll();
        contactCardRepository.deleteAll();
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();
    }

    @Test
    void unauthenticatedAccessToContactCardReturns401() throws Exception {
        mockMvc.perform(get("/api/me/contact-card"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void getContactCardWithoutCardReturnsHasCardFalse() throws Exception {
        String token = registerVerifiedAndLogin("cc-nocard@campus.edu.cn", "Str0ngPassword!", "CCNoCard");
        mockMvc.perform(get("/api/me/contact-card")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.hasCard").value(false));
    }

    @Test
    void putContactCardWithAtLeastOneFieldSucceeds() throws Exception {
        String token = registerVerifiedAndLogin("cc-putcard@campus.edu.cn", "Str0ngPassword!", "CCPutCard");
        mockMvc.perform(put("/api/me/contact-card")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"wechatId\":\"wx_test\",\"phoneNumber\":\"13900000000\",\"qqNumber\":\"10001\"}"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.hasCard").value(true))
                .andExpect(jsonPath("$.wechatId").value("wx_test"))
                .andExpect(jsonPath("$.phoneNumber").value("13900000000"))
                .andExpect(jsonPath("$.qqNumber").value("10001"));
    }

    @Test
    void putContactCardWithAllEmptyReturnsValidationFailed() throws Exception {
        String token = registerVerifiedAndLogin("cc-emptycard@campus.edu.cn", "Str0ngPassword!", "CCEmptyCard");
        mockMvc.perform(put("/api/me/contact-card")
                        .header("Authorization", "Bearer " + token)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"wechatId\":null,\"phoneNumber\":null,\"qqNumber\":null}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
    }

    @Test
    void nonParticipantCannotViewUnlockStatus() throws Exception {
        Long convId = setupConversation("cc-nonparunlock");
        String outsiderToken = registerVerifiedAndLogin("cc-outsider@campus.edu.cn", "Str0ngPassword!", "CCOutsider");
        mockMvc.perform(get("/api/me/conversations/" + convId + "/contact-unlock")
                        .header("Authorization", "Bearer " + outsiderToken))
                .andExpect(status().isForbidden());
    }

    @Test
    void closedConversationCannotConfirmUnlock() throws Exception {
        Long convId = setupConversation("cc-closedunlock");
        String requesterToken = login("cc-closedunlockreq@campus.edu.cn", "Str0ngPassword!");
        UUID requesterId = getUserId("cc-closedunlockreq@campus.edu.cn");
        contactCardRepository.save(new ContactCard(requesterId, "wx_test", "13900000001", null));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("CONVERSATION_CLOSED"));
    }

    @Test
    void singleConfirmResultsInWaitingForPeerAndNotUnlocked() throws Exception {
        Long convId = setupConversation("cc-singleconfirm");
        String requesterToken = login("cc-singleconfirmreq@campus.edu.cn", "Str0ngPassword!");
        UUID requesterId = getUserId("cc-singleconfirmreq@campus.edu.cn");
        contactCardRepository.save(new ContactCard(requesterId, "wx_req", "13900000002", null));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("WAITING_FOR_PEER"))
                .andExpect(jsonPath("$.currentUserConfirmed").value(true))
                .andExpect(jsonPath("$.peerConfirmed").value(false));

        org.junit.jupiter.api.Assertions.assertFalse(
                contactUnlockRecordRepository.existsByConversationId(convId),
                "ContactUnlockRecord must NOT exist after single confirm");
    }

    @Test
    void bothConfirmWithCardsResultsInUnlocked() throws Exception {
        Long convId = setupConversation("cc-bothconfirm");
        String publisherToken = login("cc-bothconfirmpub@campus.edu.cn", "Str0ngPassword!");
        String requesterToken = login("cc-bothconfirmreq@campus.edu.cn", "Str0ngPassword!");
        UUID publisherId = getUserId("cc-bothconfirmpub@campus.edu.cn");
        UUID requesterId = getUserId("cc-bothconfirmreq@campus.edu.cn");

        contactCardRepository.save(new ContactCard(publisherId, "wx_pub", "13900000003", null));
        contactCardRepository.save(new ContactCard(requesterId, "wx_req", "13900000004", null));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("WAITING_FOR_PEER"));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + publisherToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("UNLOCKED"))
                .andExpect(jsonPath("$.unlockedAt").isNotEmpty());

        org.junit.jupiter.api.Assertions.assertTrue(
                contactUnlockRecordRepository.existsByConversationId(convId),
                "ContactUnlockRecord must exist after both confirm with cards");
    }

    @Test
    void unlockedUserCanViewPeerContactCard() throws Exception {
        Long convId = setupConversation("cc-viewpeer");
        String publisherToken = login("cc-viewpeerpub@campus.edu.cn", "Str0ngPassword!");
        String requesterToken = login("cc-viewpeerreq@campus.edu.cn", "Str0ngPassword!");
        UUID publisherId = getUserId("cc-viewpeerpub@campus.edu.cn");
        UUID requesterId = getUserId("cc-viewpeerreq@campus.edu.cn");

        contactCardRepository.save(new ContactCard(publisherId, "wx_peer_view", "13900000005", null));
        contactCardRepository.save(new ContactCard(requesterId, "wx_req_view", "13900000006", null));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + requesterToken));
        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + publisherToken));

        mockMvc.perform(get("/api/me/conversations/" + convId + "/peer-contact-card")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.wechatId").value("wx_peer_view"))
                .andExpect(jsonPath("$.phoneNumber").value("13900000005"));
    }

    @Test
    void notUnlockedCannotViewPeerContactCard() throws Exception {
        Long convId = setupConversation("cc-notunlockedview");
        String requesterToken = login("cc-notunlockedviewreq@campus.edu.cn", "Str0ngPassword!");
        UUID requesterId = getUserId("cc-notunlockedviewreq@campus.edu.cn");
        contactCardRepository.save(new ContactCard(requesterId, "wx_req", "13900000007", null));

        mockMvc.perform(get("/api/me/conversations/" + convId + "/peer-contact-card")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("CONTACT_NOT_UNLOCKED"));
    }

    @Test
    void closedConversationCannotViewPeerContactCardEvenIfUnlocked() throws Exception {
        Long convId = setupConversation("cc-closedview");
        String publisherToken = login("cc-closedviewpub@campus.edu.cn", "Str0ngPassword!");
        String requesterToken = login("cc-closedviewreq@campus.edu.cn", "Str0ngPassword!");
        UUID publisherId = getUserId("cc-closedviewpub@campus.edu.cn");
        UUID requesterId = getUserId("cc-closedviewreq@campus.edu.cn");

        contactCardRepository.save(new ContactCard(publisherId, "wx_pub", "13900000008", null));
        contactCardRepository.save(new ContactCard(requesterId, "wx_req", "13900000009", null));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + requesterToken));
        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + publisherToken));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/close")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isOk());

        mockMvc.perform(get("/api/me/conversations/" + convId + "/peer-contact-card")
                        .header("Authorization", "Bearer " + requesterToken))
                .andExpect(status().isForbidden())
                .andExpect(jsonPath("$.code").value("CONTACT_UNLOCK_NOT_AVAILABLE"));
    }

    @Test
    void unlockedAllowsSixStarReview() throws Exception {
        Long convId = setupConversationWithMessages("cc-sixstar");
        String publisherToken = login("cc-sixstarpub@campus.edu.cn", "Str0ngPassword!");
        String requesterToken = login("cc-sixstarreq@campus.edu.cn", "Str0ngPassword!");
        UUID publisherId = getUserId("cc-sixstarpub@campus.edu.cn");
        UUID requesterId = getUserId("cc-sixstarreq@campus.edu.cn");

        contactCardRepository.save(new ContactCard(publisherId, "wx_pub", "13900000010", null));
        contactCardRepository.save(new ContactCard(requesterId, "wx_req", "13900000011", null));

        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + requesterToken));
        mockMvc.perform(post("/api/me/conversations/" + convId + "/contact-unlock/confirm")
                        .header("Authorization", "Bearer " + publisherToken));

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + convId + ",\"revieweeId\":\"" + publisherId + "\",\"rating\":6,\"tags\":[\"体验很好\"],\"comment\":\"unlocked six star\"}"))
                .andExpect(status().isCreated())
                .andExpect(jsonPath("$.rating").value(6));
    }

    @Test
    void notUnlockedRejectsSixStarReview() throws Exception {
        Long convId = setupConversationWithMessages("cc-fivesixstar");
        String publisherToken = login("cc-fivesixstarpub@campus.edu.cn", "Str0ngPassword!");
        String requesterToken = login("cc-fivesixstarreq@campus.edu.cn", "Str0ngPassword!");
        UUID publisherId = getUserId("cc-fivesixstarpub@campus.edu.cn");

        mockMvc.perform(post("/api/me/reviews")
                        .header("Authorization", "Bearer " + requesterToken)
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"conversationId\":" + convId + ",\"revieweeId\":\"" + publisherId + "\",\"rating\":6,\"tags\":[\"体验很好\"],\"comment\":\"not unlocked six star\"}"))
                .andExpect(status().isBadRequest())
                .andExpect(jsonPath("$.code").value("VALIDATION_FAILED"));
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

    private Long setupConversationWithMessages(String prefix) throws Exception {
        Long convId = setupConversation(prefix);
        UUID publisherId = getUserId(prefix + "pub@campus.edu.cn");
        UUID requesterId = getUserId(prefix + "req@campus.edu.cn");
        for (int i = 0; i < 2; i++) {
            conversationMessageRepository.save(new ConversationMessage(convId, publisherId, "USER_TEXT", "msg", Instant.now()));
            conversationMessageRepository.save(new ConversationMessage(convId, requesterId, "USER_TEXT", "msg", Instant.now()));
        }
        return convId;
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
