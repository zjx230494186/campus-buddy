package com.campusbuddy.moderation;

import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostAdminService;
import com.campusbuddy.post.PartnerPostRepository;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

@Service
public class PostModerationService {

    public static final UUID SYSTEM_REVIEWER_ID = UUID.fromString("00000000-0000-0000-0000-000000000001");
    private static final int MAX_REJECT_REASON_LENGTH = 200;

    private final PartnerPostRepository postRepository;
    private final PartnerPostAdminService adminService;
    private final PostModerationClient moderationClient;
    private final PostModerationProperties properties;

    public PostModerationService(
            PartnerPostRepository postRepository,
            PartnerPostAdminService adminService,
            PostModerationClient moderationClient,
            PostModerationProperties properties) {
        this.postRepository = postRepository;
        this.adminService = adminService;
        this.moderationClient = moderationClient;
        this.properties = properties;
    }

    @Transactional
    public void moderateAfterSubmit(UUID postId) {
        PartnerPost post = postRepository.findById(postId).orElse(null);
        if (post == null || !properties.isEnabled() || !"PENDING_REVIEW".equals(post.getStatus())) {
            return;
        }

        try {
            PostModerationDecision decision = moderationClient.moderate(toRequest(post));
            applyDecision(postId, decision);
        } catch (RuntimeException ignored) {
            // Provider failures must not block manual review.
        }
    }

    private void applyDecision(UUID postId, PostModerationDecision decision) {
        if (decision == null || decision.decision() == null) {
            return;
        }

        String normalizedDecision = decision.decision().trim().toUpperCase();
        if ("APPROVE".equals(normalizedDecision) && decision.confidence() >= properties.getAutoApproveThreshold()) {
            adminService.approvePendingPost(SYSTEM_REVIEWER_ID, postId, Instant.now());
            return;
        }

        if ("REJECT".equals(normalizedDecision) && decision.confidence() >= properties.getAutoRejectThreshold()) {
            String reason = cleanRejectReason(decision.userVisibleReason());
            if (reason != null) {
                adminService.rejectPendingPost(SYSTEM_REVIEWER_ID, postId, reason, Instant.now());
            }
        }
    }

    private PostModerationRequest toRequest(PartnerPost post) {
        return new PostModerationRequest(
                post.getId(),
                post.getPublisherId(),
                limit(post.getSceneType()),
                limit(post.getTitle()),
                limit(post.getDescription()),
                limit(post.getTimeMode()),
                limit(post.getTimeText()),
                post.getStartAt(),
                post.getEndAt(),
                limit(post.getLocationText()),
                post.getParticipantCount(),
                limit(post.getTargetRequirement()),
                null,
                limitList(post.getTags()),
                limitMap(post.getScenePayload())
        );
    }

    private List<String> limitList(List<String> values) {
        if (values == null) {
            return null;
        }
        return values.stream().map(this::limit).toList();
    }

    private Map<String, Object> limitMap(Map<String, Object> values) {
        if (values == null) {
            return null;
        }
        Map<String, Object> limited = new LinkedHashMap<>();
        values.forEach((key, value) -> limited.put(limit(key), value instanceof String text ? limit(text) : value));
        return limited;
    }

    private String limit(String value) {
        if (value == null) {
            return null;
        }
        int max = Math.max(properties.getMaxInputChars(), 1);
        return value.length() <= max ? value : value.substring(0, max);
    }

    private String cleanRejectReason(String reason) {
        if (reason == null || reason.isBlank()) {
            return null;
        }
        String trimmed = reason.trim();
        return trimmed.length() <= MAX_REJECT_REASON_LENGTH
                ? trimmed
                : trimmed.substring(0, MAX_REJECT_REASON_LENGTH);
    }
}
