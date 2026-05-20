package com.campusbuddy.review;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.contact.ContactContextService;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.contact.Conversation;
import com.campusbuddy.contact.ConversationMessage;
import com.campusbuddy.contact.ConversationRepository;
import com.campusbuddy.contact.ConversationMessageRepository;
import com.campusbuddy.contact.ContactUnlockRecordRepository;
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
class ContactContextServiceAccessTest {

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

    private static int counter = 0;
    private UUID userA;
    private UUID userB;

    @BeforeEach
    void setUp() {
        contactUnlockRecordRepository.deleteAll();
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();

        Instant now = Instant.now();
        userA = createUser(now);
        userB = createUser(now);
    }

    private UUID createUser(Instant now) {
        String email = "access-test-" + (++counter) + "@campus.edu.cn";
        UserAccount user = new UserAccount(email, "hash", "User", now);
        userAccountRepository.save(user);
        return user.getUserId();
    }

    @Test
    void crossPackageCanCallIsParticipant() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        assertTrue(contactContextService.isParticipant(conv.getId(), userA));
    }

    @Test
    void crossPackageCanCallIsOtherParticipant() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        assertTrue(contactContextService.isOtherParticipant(conv.getId(), userA, userB));
    }

    @Test
    void crossPackageCanCallCountUserMessages() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", Instant.now()));
        assertEquals(1, contactContextService.countUserMessages(conv.getId(), userA));
    }

    @Test
    void crossPackageCanCallIsValidConversation() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", Instant.now()));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", Instant.now().plusSeconds(1)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userB, "USER_TEXT", Instant.now().plusSeconds(2)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userB, "USER_TEXT", Instant.now().plusSeconds(3)));
        assertTrue(contactContextService.isValidConversation(conv.getId()));
    }

    @Test
    void crossPackageCanCallFindConversationsByParticipant() {
        conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        assertEquals(1, contactContextService.findConversationsByParticipant(userA).size());
    }

    @Test
    void crossPackageCanCallIsContactUnlocked() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        assertFalse(contactContextService.isContactUnlocked(conv.getId()));
    }

    @Test
    void unknownMessageTypeNotCountedAsUserMessage() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", Instant.now()));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "UNKNOWN", Instant.now().plusSeconds(1)));
        assertEquals(1, contactContextService.countUserMessages(conv.getId(), userA));
    }

    @Test
    void unknownMessageTypeDoesNotMakeValidConversation() {
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", Instant.now()));
        Instant base = Instant.now();
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", base));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "UNKNOWN", base.plusSeconds(1)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userB, "USER_TEXT", base.plusSeconds(2)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userB, "UNKNOWN", base.plusSeconds(3)));
        assertFalse(contactContextService.isValidConversation(conv.getId()));
    }

    @TestConfiguration
    static class TestConfig {
    }
}
