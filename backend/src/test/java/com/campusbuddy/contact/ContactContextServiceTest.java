package com.campusbuddy.contact;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.context.annotation.Import;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;

import java.time.Instant;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class ContactContextServiceTest {

    @Autowired
    private ContactContextService contactContextService;

    @Autowired
    private ConversationRepository conversationRepository;

    @Autowired
    private ConversationMessageRepository conversationMessageRepository;

    @Autowired
    private ContactUnlockRecordRepository contactUnlockRecordRepository;

    @Autowired
    private UserAccountRepository userAccountRepository;

    private static int userCounter = 0;
    private UUID userA;
    private UUID userB;
    private UUID userC;

    @BeforeEach
    void setUp() {
        contactUnlockRecordRepository.deleteAll();
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();

        Instant now = Instant.now();
        userA = createUser(now);
        userB = createUser(now);
        userC = createUser(now);
    }

    private UUID createUser(Instant now) {
        String email = "contact-svc-" + (++userCounter) + "@campus.edu.cn";
        UserAccount user = new UserAccount(email, "hash", "User", now);
        userAccountRepository.save(user);
        return user.getUserId();
    }

    @Test
    void participant1IsRecognizedAsParticipant() {
        Conversation conv = createConversation(userA, userB);
        assertTrue(contactContextService.isParticipant(conv.getId(), userA));
    }

    @Test
    void participant2IsRecognizedAsParticipant() {
        Conversation conv = createConversation(userA, userB);
        assertTrue(contactContextService.isParticipant(conv.getId(), userB));
    }

    @Test
    void thirdPartyIsNotParticipant() {
        Conversation conv = createConversation(userA, userB);
        assertFalse(contactContextService.isParticipant(conv.getId(), userC));
    }

    @Test
    void sameUserCannotBeBothParticipants() {
        assertThrows(IllegalArgumentException.class, () -> {
            Conversation conv = new Conversation(userA, userA, "ACTIVE", Instant.now());
            conversationRepository.save(conv);
        });
    }

    @Test
    void isOtherParticipant_whenBothAreSidesOfConversation_returnsTrue() {
        Conversation conv = createConversation(userA, userB);
        assertTrue(contactContextService.isOtherParticipant(conv.getId(), userA, userB));
    }

    @Test
    void isOtherParticipant_whenRevieweeIsReviewer_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        assertFalse(contactContextService.isOtherParticipant(conv.getId(), userA, userA));
    }

    @Test
    void isOtherParticipant_whenRevieweeIsThirdParty_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        assertFalse(contactContextService.isOtherParticipant(conv.getId(), userA, userC));
    }

    @Test
    void countUserMessages_countsUserTextMessages() {
        Conversation conv = createConversation(userA, userB);
        sendMessage(conv.getId(), userA, "USER_TEXT", Instant.now());
        sendMessage(conv.getId(), userA, "USER_TEXT", Instant.now());
        assertEquals(2, contactContextService.countUserMessages(conv.getId(), userA));
    }

    @Test
    void countUserMessages_excludesSystemMessages() {
        Conversation conv = createConversation(userA, userB);
        sendMessage(conv.getId(), userA, "USER_TEXT", Instant.now());
        sendMessage(conv.getId(), userA, "SYSTEM", Instant.now());
        assertEquals(1, contactContextService.countUserMessages(conv.getId(), userA));
    }

    @Test
    void countUserMessages_excludesNonParticipantMessages() {
        Conversation conv = createConversation(userA, userB);
        sendMessage(conv.getId(), userC, "USER_TEXT", Instant.now());
        assertEquals(0, contactContextService.countUserMessages(conv.getId(), userC));
    }

    @Test
    void isValidConversation_whenBothHaveAtLeast2Messages_returnsTrue() {
        Conversation conv = createConversation(userA, userB);
        sendTwoUserMessages(conv.getId(), userA);
        sendTwoUserMessages(conv.getId(), userB);
        assertTrue(contactContextService.isValidConversation(conv.getId()));
    }

    @Test
    void isValidConversation_whenOnlyOneSideHas2Messages_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        sendTwoUserMessages(conv.getId(), userA);
        sendMessage(conv.getId(), userB, "USER_TEXT", Instant.now());
        assertFalse(contactContextService.isValidConversation(conv.getId()));
    }

    @Test
    void isValidConversation_whenBothHave1Message_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        sendMessage(conv.getId(), userA, "USER_TEXT", Instant.now());
        sendMessage(conv.getId(), userB, "USER_TEXT", Instant.now());
        assertFalse(contactContextService.isValidConversation(conv.getId()));
    }

    @Test
    void isValidConversation_whenOneSideHas0Messages_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        sendTwoUserMessages(conv.getId(), userA);
        assertFalse(contactContextService.isValidConversation(conv.getId()));
    }

    @Test
    void isValidConversation_whenOnlySystemMessages_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        sendMessage(conv.getId(), null, "SYSTEM", Instant.now());
        sendMessage(conv.getId(), null, "SYSTEM", Instant.now());
        assertFalse(contactContextService.isValidConversation(conv.getId()));
    }

    @Test
    void isContactUnlocked_whenRecordExists_returnsTrue() {
        Conversation conv = createConversation(userA, userB);
        createUnlockRecord(conv.getId());
        assertTrue(contactContextService.isContactUnlocked(conv.getId()));
    }

    @Test
    void isContactUnlocked_whenNoRecord_returnsFalse() {
        Conversation conv = createConversation(userA, userB);
        assertFalse(contactContextService.isContactUnlocked(conv.getId()));
    }

    @Test
    void contactUnlockRecord_uniquePerConversation() {
        Conversation conv = createConversation(userA, userB);
        createUnlockRecord(conv.getId());
        assertThrows(Exception.class, () -> createUnlockRecord(conv.getId()));
    }

    @Test
    void findConversationsByParticipant_returnsConversationsForUser() {
        Conversation conv1 = createConversation(userA, userB);
        Conversation conv2 = createConversation(userA, userC);
        var conversations = contactContextService.findConversationsByParticipant(userA);
        assertEquals(2, conversations.size());
        assertTrue(conversations.stream().anyMatch(c -> c.getId().equals(conv1.getId())));
        assertTrue(conversations.stream().anyMatch(c -> c.getId().equals(conv2.getId())));
    }

    private Conversation createConversation(UUID p1, UUID p2) {
        Conversation conv = new Conversation(p1, p2, "ACTIVE", Instant.now());
        return conversationRepository.save(conv);
    }

    private void sendMessage(Long convId, UUID senderId, String type, Instant createdAt) {
        ConversationMessage msg = new ConversationMessage(convId, senderId, type, createdAt);
        conversationMessageRepository.save(msg);
    }

    private void sendTwoUserMessages(Long convId, UUID senderId) {
        Instant base = Instant.now();
        sendMessage(convId, senderId, "USER_TEXT", base);
        sendMessage(convId, senderId, "USER_TEXT", base.plusSeconds(1));
    }

    private void createUnlockRecord(Long convId) {
        ContactUnlockRecord record = new ContactUnlockRecord(convId, Instant.now());
        contactUnlockRecordRepository.save(record);
    }

    @TestConfiguration
    static class TestConfig {
    }
}
