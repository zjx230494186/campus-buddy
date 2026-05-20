package com.campusbuddy.review;

import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import com.campusbuddy.contact.ContactContextService;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.List;
import java.util.Set;
import java.util.UUID;

@Service
public class ReviewService {

    private static final Set<String> PRESET_TAGS = Set.of(
            "守时", "沟通顺畅", "配合度高", "认真负责", "体验很好",
            "迟到", "失联", "临时变卦", "配合度低", "体验不佳"
    );

    private final ReviewRepository reviewRepository;
    private final ContactContextService contactContextService;
    private final UserAccountRepository userAccountRepository;

    public ReviewService(ReviewRepository reviewRepository,
                         ContactContextService contactContextService,
                         UserAccountRepository userAccountRepository) {
        this.reviewRepository = reviewRepository;
        this.contactContextService = contactContextService;
        this.userAccountRepository = userAccountRepository;
    }

    @Transactional
    public ReviewResponse createReview(UUID reviewerId, CreateReviewRequest request) {
        var user = userAccountRepository.findById(reviewerId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "AUTHENTICATION_STATUS_REQUIRED", "User authentication status insufficient", null));
        if (!"VERIFIED".equals(user.getAuthenticationStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "AUTHENTICATION_STATUS_REQUIRED", "User must be VERIFIED to submit reviews", null);
        }

        if (!contactContextService.isParticipant(request.conversationId(), reviewerId)) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "CONVERSATION_NOT_REVIEWABLE", "You are not a participant of this conversation", null);
        }
        if (!contactContextService.isOtherParticipant(request.conversationId(), reviewerId, request.revieweeId())) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "CONVERSATION_NOT_REVIEWABLE", "Reviewee is not the other participant", null);
        }
        if (!contactContextService.isValidConversation(request.conversationId())) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "CONVERSATION_NOT_REVIEWABLE", "Conversation does not meet valid conversation criteria", null);
        }
        if (reviewRepository.existsByConversationIdAndReviewerIdAndRevieweeId(request.conversationId(), reviewerId, request.revieweeId())) {
            throw new ApiException(HttpStatus.CONFLICT, "REVIEW_ALREADY_EXISTS", "Review already exists for this conversation", null);
        }

        int maxRating = contactContextService.isContactUnlocked(request.conversationId()) ? 6 : 5;
        if (request.rating() < 1 || request.rating() > maxRating) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Rating must be between 1 and " + maxRating, null);
        }

        if (request.reviewTags() != null) {
            for (String tag : request.reviewTags()) {
                if (!PRESET_TAGS.contains(tag)) {
                    throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Tag '" + tag + "' is not a preset tag", null);
                }
            }
        }

        String tagsStr = request.reviewTags() == null || request.reviewTags().isEmpty()
                ? null : String.join(",", request.reviewTags());
        Instant now = Instant.now();
        Review review = new Review(request.conversationId(), reviewerId, request.revieweeId(), request.rating(), tagsStr, now);
        Review saved = reviewRepository.save(review);
        return toResponse(saved);
    }

    @Transactional
    public ReviewResponse updateReview(UUID reviewerId, Long reviewId, UpdateReviewRequest request) {
        Review review = reviewRepository.findByIdAndReviewerId(reviewId, reviewerId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "REVIEW_NOT_FOUND", "Review not found or you are not the reviewer", null));

        if (review.isModifiedOnce()) {
            throw new ApiException(HttpStatus.CONFLICT, "REVIEW_MODIFICATION_LIMIT_EXCEEDED", "Review can only be modified once", null);
        }
        if (Instant.now().isAfter(review.getCreatedAt().plusSeconds(24 * 3600))) {
            throw new ApiException(HttpStatus.CONFLICT, "REVIEW_MODIFICATION_EXPIRED", "Modification window has expired (24 hours)", null);
        }

        int maxRating = contactContextService.isContactUnlocked(review.getConversationId()) ? 6 : 5;
        if (request.rating() < 1 || request.rating() > maxRating) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Rating must be between 1 and " + maxRating, null);
        }

        if (request.reviewTags() != null) {
            for (String tag : request.reviewTags()) {
                if (!PRESET_TAGS.contains(tag)) {
                    throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Tag '" + tag + "' is not a preset tag", null);
                }
            }
        }

        String tagsStr = request.reviewTags() == null || request.reviewTags().isEmpty()
                ? null : String.join(",", request.reviewTags());
        review.update(request.rating(), tagsStr);
        Review saved = reviewRepository.save(review);
        return toResponse(saved);
    }

    private ReviewResponse toResponse(Review review) {
        return new ReviewResponse(
                review.getId(),
                review.getConversationId(),
                review.getReviewerId(),
                review.getRevieweeId(),
                review.getRating(),
                review.getReviewTags() == null ? List.of() : List.of(review.getReviewTags().split(",")),
                review.getStatus(),
                review.isModifiedOnce(),
                review.getCreatedAt(),
                review.getUpdatedAt()
        );
    }

    public static Set<String> getPresetTags() {
        return PRESET_TAGS;
    }

    public record CreateReviewRequest(Long conversationId, UUID revieweeId, int rating, java.util.List<String> reviewTags) {}
    public record UpdateReviewRequest(int rating, java.util.List<String> reviewTags) {}
    public record ReviewResponse(Long id, Long conversationId, UUID reviewerId, UUID revieweeId, int rating, java.util.List<String> reviewTags, String status, boolean modifiedOnce, Instant createdAt, Instant updatedAt) {}
}
