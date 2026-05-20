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

        List<Conversation> validConversations = allConversations.stream()
                .filter(conv -> contactContextService.isValidConversation(conv.getId()))
                .toList();

        long realConversationCount = validConversations.size();

        Map<Long, Review> reviewByConversation = activeReviews.stream()
                .collect(Collectors.toMap(Review::getConversationId, r -> r, (a, b) -> a));

        long totalRating = VIRTUAL_BASELINE_TOTAL;
        long sampleCount = VIRTUAL_BASELINE_COUNT;

        for (Conversation conv : validConversations) {
            Review review = reviewByConversation.get(conv.getId());
            if (review != null) {
                totalRating += review.getRating();
            } else {
                totalRating += DEFAULT_RATING;
            }
            sampleCount++;
        }

        Set<Long> validConversationIds = validConversations.stream()
                .map(Conversation::getId)
                .collect(Collectors.toSet());

        Map<String, Long> tagCounts = new LinkedHashMap<>();
        for (Review review : activeReviews) {
            if (validConversationIds.contains(review.getConversationId())
                    && review.getReviewTags() != null && !review.getReviewTags().isBlank()) {
                for (String tag : review.getReviewTags().split(",")) {
                    String trimmed = tag.trim();
                    if (!trimmed.isEmpty()) {
                        tagCounts.merge(trimmed, 1L, Long::sum);
                    }
                }
            }
        }

        double averageRating = sampleCount > 0 ? (double) totalRating / sampleCount : 0.0;

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

    public record PublicCreditSummaryResponse(
            UUID userId,
            double averageRating,
            long realConversationCount,
            long ratingSampleCount,
            List<CreditSummaryTagResponse> topTags,
            Instant updatedAt
    ) {}

    public PublicCreditSummaryResponse toPublicResponse(CreditSummaryResponse response) {
        return new PublicCreditSummaryResponse(
                response.userId(),
                response.averageRating(),
                response.realConversationCount(),
                response.ratingSampleCount(),
                response.topTags(),
                response.updatedAt()
        );
    }

    public record CreditSummaryTagResponse(String tagName, long count) {}
}
