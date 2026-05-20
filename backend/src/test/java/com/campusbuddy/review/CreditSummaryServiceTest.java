package com.campusbuddy.review;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.contact.*;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.context.annotation.Import;

import java.time.Instant;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class CreditSummaryServiceTest {

    @Autowired private CreditSummaryService creditSummaryService;
    @Autowired private ReviewRepository reviewRepository;
    @Autowired private ConversationRepository conversationRepository;
    @Autowired private ConversationMessageRepository conversationMessageRepository;
    @Autowired private UserAccountRepository userAccountRepository;

    private static int counter = 0;

    @BeforeEach
    void cleanUp() {
        reviewRepository.deleteAll();
        conversationMessageRepository.deleteAll();
        conversationRepository.deleteAll();
    }

    private UUID createRealUser(Instant now) {
        String email = "credit-test-" + (++counter) + "@campus.edu.cn";
        UserAccount user = new UserAccount(email, "hash", "User", now);
        userAccountRepository.save(user);
        return user.getUserId();
    }

    @Test
    void newAccount_hasBaselineCreditSummary() {
        Instant now = Instant.now();
        UUID userId = createRealUser(now);
        var summary = creditSummaryService.getCreditSummary(userId, true);
        assertEquals(3.5, summary.averageRating(), 0.01);
        assertEquals(0, summary.realConversationCount());
        assertEquals(6, summary.ratingSampleCount());
        assertTrue(summary.topTags().isEmpty());
    }

    @Test
    void oneValidConversationNoReview_default4Star() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);
        createValidConversation(userA, userB, now);

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(1, summaryB.realConversationCount());
        assertEquals(7, summaryB.ratingSampleCount());
        assertEquals(3.6, summaryB.averageRating(), 0.01);
        assertTrue(summaryB.topTags().isEmpty());
    }

    @Test
    void explicitReviewAffectsSummary() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);
        Conversation conv = createValidConversation(userA, userB, now);

        Review review = new Review(conv.getId(), userA, userB, 5, "守时", now);
        reviewRepository.save(review);

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(3.7, summaryB.averageRating(), 0.01);
        assertEquals(1, summaryB.topTags().size());
        assertEquals("守时", summaryB.topTags().get(0).tagName());
        assertEquals(1, summaryB.topTags().get(0).count());
    }

    @Test
    void modifyReview_recalculatesSummary() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);
        Conversation conv = createValidConversation(userA, userB, now);

        Review review = new Review(conv.getId(), userA, userB, 5, "守时", now);
        reviewRepository.save(review);

        review.update(2, "迟到");
        reviewRepository.save(review);

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(3.3, summaryB.averageRating(), 0.01);
        assertEquals(1, summaryB.topTags().size());
        assertEquals("迟到", summaryB.topTags().get(0).tagName());
    }

    @Test
    void topTagsLimitedTo5() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);

        String[] tags = {"守时", "沟通顺畅", "配合度高", "认真负责", "体验很好", "迟到"};
        for (int i = 0; i < tags.length; i++) {
            Conversation conv = createValidConversation(userA, userB, now.plusSeconds(i * 100));
            Review review = new Review(conv.getId(), userA, userB, 4, tags[i], now.plusSeconds(i * 100 + 1));
            reviewRepository.save(review);
        }

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(5, summaryB.topTags().size());
    }

    @Test
    void default4StarProducesNoTags() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);
        createValidConversation(userA, userB, now);

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertTrue(summaryB.topTags().isEmpty());
    }

    @Test
    void disputedReviewCountDefaultsTo0() {
        Instant now = Instant.now();
        UUID userId = createRealUser(now);
        var summary = creditSummaryService.getCreditSummary(userId, true);
        assertEquals(0, summary.disputedReviewCount());
    }

    @Test
    void invalidConversationNotCountedInSummary() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);
        Conversation conv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", now));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), userA, "USER_TEXT", now));

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(0, summaryB.realConversationCount());
        assertEquals(6, summaryB.ratingSampleCount());
        assertEquals(3.5, summaryB.averageRating(), 0.01);

        createValidConversation(userA, userB, now.plusSeconds(100));
        var summaryB2 = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(1, summaryB2.realConversationCount());
        assertEquals(7, summaryB2.ratingSampleCount());
        assertEquals(3.6, summaryB2.averageRating(), 0.01);
    }

    @Test
    void reviewOnInvalidConversationNotCountedInSummary() {
        Instant now = Instant.now();
        UUID userA = createRealUser(now);
        UUID userB = createRealUser(now);
        Conversation invalidConv = conversationRepository.save(new Conversation(userA, userB, "ACTIVE", now));
        conversationMessageRepository.save(new ConversationMessage(invalidConv.getId(), userA, "USER_TEXT", now));

        Review dirtyReview = new Review(invalidConv.getId(), userA, userB, 5, "守时", now);
        reviewRepository.save(dirtyReview);

        var summaryB = creditSummaryService.getCreditSummary(userB, false);
        assertEquals(0, summaryB.realConversationCount());
        assertEquals(6, summaryB.ratingSampleCount());
        assertEquals(3.5, summaryB.averageRating(), 0.01);
        assertTrue(summaryB.topTags().isEmpty());
    }

    private Conversation createValidConversation(UUID p1, UUID p2, Instant base) {
        Conversation conv = conversationRepository.save(new Conversation(p1, p2, "ACTIVE", base));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p1, "USER_TEXT", base.plusSeconds(1)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p1, "USER_TEXT", base.plusSeconds(2)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p2, "USER_TEXT", base.plusSeconds(3)));
        conversationMessageRepository.save(new ConversationMessage(conv.getId(), p2, "USER_TEXT", base.plusSeconds(4)));
        return conv;
    }

    @TestConfiguration
    static class TestConfig {
    }
}
