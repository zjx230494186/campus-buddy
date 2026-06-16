package com.campusbuddy.post;

import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.*;

@Service
public class PartnerPostAdminService {

    private static final int MAX_PUBLISHED_PER_USER = 10;

    private final PartnerPostRepository postRepository;
    private final UserAccountRepository userAccountRepository;

    PartnerPostAdminService(PartnerPostRepository postRepository, UserAccountRepository userAccountRepository) {
        this.postRepository = postRepository;
        this.userAccountRepository = userAccountRepository;
    }

    public ReviewQueueResponse reviewQueue(int page, int size) {
        PageRequest pageable = PageRequest.of(page, size);
        Page<PartnerPost> postPage = postRepository.findByStatus("PENDING_REVIEW", pageable);
        List<ReviewQueueItem> items = postPage.getContent().stream().map(this::toQueueItem).toList();
        return new ReviewQueueResponse(items, postPage.getNumber(), postPage.getSize(), postPage.getTotalElements(), postPage.getTotalPages());
    }

    public AdminPostDetailResponse getPostDetail(UUID postId) {
        PartnerPost post = postRepository.findById(postId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist"));

        UserAccount publisher = userAccountRepository.findById(post.getPublisherId()).orElse(null);
        String publisherDisplayName = publisher != null ? publisher.getDisplayName() : null;
        String publisherAuthenticationStatus = publisher != null ? publisher.getAuthenticationStatus() : null;

        return new AdminPostDetailResponse(
                post.getId().toString(),
                post.getPublisherId().toString(),
                publisherDisplayName,
                publisherAuthenticationStatus,
                post.getSceneType(),
                post.getStatus(),
                post.getTitle(),
                post.getDescription(),
                post.getTimeMode(),
                post.getTimeText(),
                post.getStartAt() != null ? post.getStartAt().toString() : null,
                post.getEndAt() != null ? post.getEndAt().toString() : null,
                post.getLocationText(),
                post.getParticipantCount(),
                post.getTargetRequirement(),
                post.getTags(),
                post.getScenePayload(),
                post.getRejectReason(),
                post.getReviewedBy() != null ? post.getReviewedBy().toString() : null,
                post.getReviewedAt() != null ? post.getReviewedAt().toString() : null,
                post.getPublishedAt() != null ? post.getPublishedAt().toString() : null,
                post.getCreatedAt().toString(),
                post.getUpdatedAt().toString()
        );
    }

    @Transactional
    public AdminPostDetailResponse reviewPost(UUID adminId, UUID postId, ReviewDecisionRequest request) {
        validateReviewRequest(request);
        Instant now = Instant.now();
        if ("APPROVE".equals(request.decision())) {
            return approvePendingPost(adminId, postId, now);
        }
        return rejectPendingPost(adminId, postId, request.reason(), now);
    }

    @Transactional
    public AdminPostDetailResponse approvePendingPost(UUID reviewerId, UUID postId, Instant now) {
        PartnerPost post = postRepository.findById(postId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist"));

        if (!"PENDING_REVIEW".equals(post.getStatus())) {
            throw new ApiException(HttpStatus.CONFLICT, "POST_STATUS_CONFLICT",
                    "Only PENDING_REVIEW posts can be reviewed", "current status: " + post.getStatus());
        }

        long publishedCount = postRepository.countByPublisherIdAndStatus(post.getPublisherId(), "PUBLISHED");
        if (publishedCount >= MAX_PUBLISHED_PER_USER) {
            throw new ApiException(HttpStatus.CONFLICT, "PUBLISHED_POST_LIMIT_EXCEEDED",
                    "Publisher already has " + MAX_PUBLISHED_PER_USER + " published posts",
                    "current published count: " + publishedCount);
        }

        post.setStatus("PUBLISHED");
        post.setReviewedBy(reviewerId);
        post.setReviewedAt(now);
        post.setPublishedAt(now);
        post.setRejectReason(null);
        post.setUpdatedAt(now);

        postRepository.save(post);
        return getPostDetail(postId);
    }

    @Transactional
    public AdminPostDetailResponse rejectPendingPost(UUID reviewerId, UUID postId, String reason, Instant now) {
        if (reason == null || reason.isBlank() || reason.length() > 200) {
            Map<String, String> errors = new LinkedHashMap<>();
            errors.put("reason", reason == null || reason.isBlank() ? "is required for REJECT" : "must not exceed 200 characters");
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Validation failed", errors);
        }

        PartnerPost post = postRepository.findById(postId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist"));

        if (!"PENDING_REVIEW".equals(post.getStatus())) {
            throw new ApiException(HttpStatus.CONFLICT, "POST_STATUS_CONFLICT",
                    "Only PENDING_REVIEW posts can be reviewed", "current status: " + post.getStatus());
        }

        post.setStatus("REJECTED");
        post.setReviewedBy(reviewerId);
        post.setReviewedAt(now);
        post.setRejectReason(reason);
        post.setPublishedAt(null);
        post.setUpdatedAt(now);

        postRepository.save(post);
        return getPostDetail(postId);
    }

    private void validateReviewRequest(ReviewDecisionRequest request) {
        Map<String, String> errors = new LinkedHashMap<>();

        if (request.decision() == null || (!"APPROVE".equals(request.decision()) && !"REJECT".equals(request.decision()))) {
            errors.put("decision", "must be APPROVE or REJECT");
        }

        if ("REJECT".equals(request.decision()) && (request.reason() == null || request.reason().isBlank())) {
            errors.put("reason", "is required for REJECT");
        }

        if (request.reason() != null && request.reason().length() > 200) {
            errors.put("reason", "must not exceed 200 characters");
        }

        if (!errors.isEmpty()) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Validation failed", errors);
        }
    }

    private ReviewQueueItem toQueueItem(PartnerPost post) {
        UserAccount publisher = userAccountRepository.findById(post.getPublisherId()).orElse(null);
        String publisherDisplayName = publisher != null ? publisher.getDisplayName() : null;
        String summary = post.getDescription() != null && post.getDescription().length() > 50
                ? post.getDescription().substring(0, 50) + "..."
                : post.getDescription();

        return new ReviewQueueItem(
                post.getId().toString(),
                post.getPublisherId().toString(),
                publisherDisplayName,
                post.getSceneType(),
                post.getStatus(),
                post.getTitle(),
                summary,
                post.getTimeText(),
                post.getLocationText(),
                post.getUpdatedAt().toString()
        );
    }

    public record ReviewQueueItem(
            String postId, String publisherId, String publisherDisplayName,
            String sceneType, String status, String title, String summary,
            String timeText, String locationText, String updatedAt
    ) {}

    public record ReviewQueueResponse(
            List<ReviewQueueItem> items, int page, int size, long totalElements, int totalPages
    ) {}

    public record AdminPostDetailResponse(
            String postId, String publisherId, String publisherDisplayName,
            String publisherAuthenticationStatus, String sceneType, String status,
            String title, String description, String timeMode, String timeText,
            String startAt, String endAt, String locationText, Integer participantCount,
            String targetRequirement, List<String> tags, Map<String, Object> scenePayload,
            String rejectReason, String reviewedBy, String reviewedAt,
            String publishedAt, String createdAt, String updatedAt
    ) {}

    public record ReviewDecisionRequest(String decision, String reason) {}
}
