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
class ContactPersistenceIntegrationTest {

    @Autowired
    private ConversationRepository conversationRepository;

    @Autowired
    private ConversationMessageRepository conversationMessageRepository;

    @Autowired
    private ContactUnlockRecordRepository contactUnlockRecordRepository;

    @Autowired
    private UserAccountRepository userAccountRepository;

    private static int counter = 0;

    @BeforeEach
    void cleanUp() {
        contactUnlockRecordRepository.deleteAll();
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();
    }

    private UUID createRealUser(Instant now) {
        String email = "persistence-contact-" + (++counter) + "@campus.edu.cn";
        UserAccount user = new UserAccount(email, "hash", "User", now);
        userAccountRepository.save(user);
        return user.getUserId();
    }

    @Test
    void conversationCanBeSavedAndFound() {
        Instant now = Instant.now();
        UUID p1 = createRealUser(now);
        UUID p2 = createRealUser(now);
        Conversation conv = new Conversation(p1, p2, "ACTIVE", now);
        Conversation saved = conversationRepository.save(conv);
        assertNotNull(saved.getId());
        assertTrue(conversationRepository.findById(saved.getId()).isPresent());
    }

    @Test
    void conversationRejectsSameParticipants() {
        Instant now = Instant.now();
        UUID p1 = createRealUser(now);
        assertThrows(Exception.class, () -> {
            Conversation conv = new Conversation(p1, p1, "ACTIVE", now);
            conversationRepository.save(conv);
        });
    }

    @Test
    void conversationMessageCanBeSavedAndCounted() {
        Instant now = Instant.now();
        UUID p1 = createRealUser(now);
        UUID p2 = createRealUser(now);
        Conversation conv = conversationRepository.save(new Conversation(p1, p2, "ACTIVE", now));

        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p1, "USER_TEXT", now));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p1, "USER_TEXT", now.plusSeconds(1)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), null, "SYSTEM", now.plusSeconds(2)));

        long userCount = conversationMessageRepository.countByConversationIdAndSenderIdAndMessageTypeNot(
                conv.getId(), p1, "SYSTEM");
        assertEquals(2, userCount);
    }

    @Test
    void contactUnlockRecordCanBeSaved() {
        Instant now = Instant.now();
        UUID p1 = createRealUser(now);
        UUID p2 = createRealUser(now);
        Conversation conv = conversationRepository.save(new Conversation(p1, p2, "ACTIVE", now));

        ContactUnlockRecord record = new ContactUnlockRecord(conv.getId(), now);
        ContactUnlockRecord saved = contactUnlockRecordRepository.save(record);
        assertNotNull(saved.getId());
        assertTrue(contactUnlockRecordRepository.existsByConversationId(conv.getId()));
    }

    @Test
    void contactUnlockRecordUniquePerConversation() {
        Instant now = Instant.now();
        UUID p1 = createRealUser(now);
        UUID p2 = createRealUser(now);
        Conversation conv = conversationRepository.save(new Conversation(p1, p2, "ACTIVE", now));

        contactUnlockRecordRepository.save(new ContactUnlockRecord(conv.getId(), now));
        assertThrows(Exception.class, () ->
                contactUnlockRecordRepository.save(new ContactUnlockRecord(conv.getId(), now)));
    }

    @Test
    void conversationClosedStatusCanBePersisted() {
        Instant now = Instant.now();
        UUID p1 = createRealUser(now);
        UUID p2 = createRealUser(now);
        Conversation conv = new Conversation(p1, p2, "CLOSED", now);
        Conversation saved = conversationRepository.save(conv);
        assertEquals("CLOSED", saved.getStatus());
    }

    @TestConfiguration
    static class TestConfig {
    }
}
