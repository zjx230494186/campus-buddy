package com.campusbuddy.review;

import com.campusbuddy.contact.ContactContextService;
import com.campusbuddy.contact.Conversation;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.*;
import java.util.stream.Collectors;

@Service
public class CreditSummaryService {

    private static final int VIRTUAL_BASELINE_COUNT = 6;
    private static final int VIRTUAL_BASELINE_TOTAL = 21;
    private static final int DEFAULT_RATING = 4;
    private static final int MAX_TOP_TAGS = 5;

    private final ReviewRepository reviewRepository;
    private final ContactContextService contactContextService;

    public CreditSummaryService(ReviewRepository reviewRepository, ContactContextService contactContextService) {
        this.reviewRepository = reviewRepository;
        this.contactContextService = contactContextService;
    }

    @Transactional(readOnly = true)
    public CreditSummaryResponse getCreditSummary(UUID userId, boolean isSelf) {
        List<Review> activeReviews = reviewRepository.findByRevieweeIdAndStatus(userId, "ACTIVE");
        List<Conversation> allConversations = contactContextService.findConversationsByParticipant(userId);

        long realConversationCount = allConversations.size();

        Map<Long, Review> reviewByConversation = activeReviews.stream()
                .collect(Collectors.toMap(Review::getConversationId, r -> r, (a, b) -> a));

        long totalRating = VIRTUAL_BASELINE_TOTAL;
        long sampleCount = VIRTUAL_BASELINE_COUNT;

        for (Conversation conv : allConversations) {
            Review review = reviewByConversation.get(conv.getId());
            if (review != null) {
                totalRating += review.getRating();
            } else {
                totalRating += DEFAULT_RATING;
            }
            sampleCount++;
        }

        double averageRating = sampleCount > 0 ? (double) totalRating / sampleCount : 0.0;

        Map<String, Long> tagCounts = new LinkedHashMap<>();
        for (Review review : activeReviews) {
            if (review.getReviewTags() != null && !review.getReviewTags().isBlank()) {
                for (String tag : review.getReviewTags().split(",")) {
                    String trimmed = tag.trim();
                    if (!trimmed.isEmpty()) {
                        tagCounts.merge(trimmed, 1L, Long::sum);
                    }
                }
            }
        }

        List<CreditSummaryTagResponse> topTags = tagCounts.entrySet().stream()
                .sorted(Map.Entry.<String, Long>comparingByValue().reversed()
                        .thenComparing(Map.Entry::getKey))
                .limit(MAX_TOP_TAGS)
                .map(e -> new CreditSummaryTagResponse(e.getKey(), e.getValue()))
                .toList();

        return new CreditSummaryResponse(
                userId,
                Math.round(averageRating * 10) / 10.0,
                realConversationCount,
                sampleCount,
                topTags,
                0,
                Instant.now()
        );
    }

    public record CreditSummaryResponse(
            UUID userId,
            double averageRating,
            long realConversationCount,
            long ratingSampleCount,
            List<CreditSummaryTagResponse> topTags,
            int disputedReviewCount,
            Instant updatedAt
    ) {}

    public record CreditSummaryTagResponse(String tagName, long count) {}
}
