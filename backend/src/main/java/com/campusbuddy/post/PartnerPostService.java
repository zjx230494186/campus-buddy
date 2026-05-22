package com.campusbuddy.post;

import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import com.fasterxml.jackson.databind.ObjectMapper;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.*;

@Service
public class PartnerPostService {

    private static final Set<String> VALID_SCENE_TYPES = Set.of("MEAL", "STUDY", "SPORT", "COURSE_TEAM", "INNOVATION_PROJECT");
    private static final Set<String> VALID_TIME_MODES = Set.of("EXACT_TIME", "TIME_RANGE", "TEXT_PREFERENCE");

    private final PartnerPostRepository postRepository;
    private final UserAccountRepository userAccountRepository;

    PartnerPostService(PartnerPostRepository postRepository, UserAccountRepository userAccountRepository) {
        this.postRepository = postRepository;
        this.userAccountRepository = userAccountRepository;
    }

    @Transactional
    public PostResponse createDraft(UUID userId, CreateDraftRequest request) {
        requireVerified(userId);
        validateDraftFields(request);

        Instant now = Instant.now();
        PartnerPost post = new PartnerPost(userId, "DRAFT", now);
        applyDraftFields(post, request);

        postRepository.save(post);
        return toResponse(post);
    }

    @Transactional
    public PostResponse updateDraft(UUID userId, UUID postId, UpdateDraftRequest request) {
        requireVerified(userId);
        validateDraftFields(request);

        PartnerPost post = postRepository.findByIdAndPublisherId(postId, userId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist or not owned by user"));

        if (!"DRAFT".equals(post.getStatus())) {
            throw new ApiException(HttpStatus.CONFLICT, "POST_STATUS_CONFLICT",
                    "Cannot update non-draft post", "current status: " + post.getStatus());
        }

        applyDraftFields(post, request);
        post.setUpdatedAt(Instant.now());
        postRepository.save(post);
        return toResponse(post);
    }

    public PostListResponse listMyPosts(UUID userId, String status, int page, int size) {
        Page<PartnerPost> postPage;
        PageRequest pageable = PageRequest.of(page, size);
        if (status != null && !status.isBlank()) {
            postPage = postRepository.findByPublisherIdAndStatus(userId, status, pageable);
        } else {
            postPage = postRepository.findByPublisherId(userId, pageable);
        }
        List<PostResponse> items = postPage.getContent().stream().map(this::toResponse).toList();
        return new PostListResponse(items, postPage.getNumber(), postPage.getSize(), postPage.getTotalElements(), postPage.getTotalPages());
    }

    public PostResponse getMyPost(UUID userId, UUID postId) {
        PartnerPost post = postRepository.findByIdAndPublisherId(postId, userId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist or not owned by user"));
        return toResponse(post);
    }

    private void requireVerified(UUID userId) {
        userAccountRepository.findById(userId).ifPresentOrElse(
                account -> {
                    if (!"VERIFIED".equals(account.getAuthenticationStatus())) {
                        throw new ApiException(HttpStatus.FORBIDDEN, "AUTHENTICATION_STATUS_REQUIRED",
                                "Verified authentication status required", "current: " + account.getAuthenticationStatus());
                    }
                },
                () -> { throw new ApiException(HttpStatus.UNAUTHORIZED, "UNAUTHORIZED", "User not found", "invalid userId"); }
        );
    }

    private void validateDraftFields(DraftFields fields) {
        Map<String, String> errors = new LinkedHashMap<>();

        if (fields.sceneType() != null && !VALID_SCENE_TYPES.contains(fields.sceneType())) {
            errors.put("sceneType", "must be one of: MEAL, STUDY, SPORT, COURSE_TEAM, INNOVATION_PROJECT");
        }
        if (fields.timeMode() != null && !VALID_TIME_MODES.contains(fields.timeMode())) {
            errors.put("timeMode", "must be one of: EXACT_TIME, TIME_RANGE, TEXT_PREFERENCE");
        }
        if (fields.title() != null && fields.title().length() > 40) {
            errors.put("title", "must not exceed 40 characters");
        }
        if (fields.description() != null && fields.description().length() > 500) {
            errors.put("description", "must not exceed 500 characters");
        }
        if (fields.locationText() != null && fields.locationText().length() > 80) {
            errors.put("locationText", "must not exceed 80 characters");
        }
        if (fields.targetRequirement() != null && fields.targetRequirement().length() > 120) {
            errors.put("targetRequirement", "must not exceed 120 characters");
        }
        if (fields.contactPreference() != null && fields.contactPreference().length() > 80) {
            errors.put("contactPreference", "must not exceed 80 characters");
        }
        if (fields.participantCount() != null && (fields.participantCount() < 1 || fields.participantCount() > 20)) {
            errors.put("participantCount", "must be between 1 and 20");
        }
        if (fields.tags() != null) {
            if (fields.tags().size() > 8) {
                errors.put("tags", "must have at most 8 items");
            }
            for (int i = 0; i < fields.tags().size(); i++) {
                String tag = fields.tags().get(i);
                if (tag == null || tag.length() < 1 || tag.length() > 12) {
                    errors.put("tags[" + i + "]", "each tag must be 1-12 characters");
                    break;
                }
            }
        }
        if (fields.startAt() != null && fields.endAt() != null && !fields.endAt().isAfter(fields.startAt())) {
            errors.put("endAt", "must be after startAt");
        }

        if (!errors.isEmpty()) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Validation failed", errors);
        }
    }

    private void applyDraftFields(PartnerPost post, DraftFields fields) {
        if (fields.sceneType() != null) post.setSceneType(fields.sceneType());
        if (fields.title() != null) post.setTitle(fields.title());
        if (fields.description() != null) post.setDescription(fields.description());
        if (fields.timeMode() != null) post.setTimeMode(fields.timeMode());
        if (fields.timeText() != null) post.setTimeText(fields.timeText());
        if (fields.startAt() != null) post.setStartAt(fields.startAt());
        if (fields.endAt() != null) post.setEndAt(fields.endAt());
        if (fields.locationText() != null) post.setLocationText(fields.locationText());
        if (fields.participantCount() != null) post.setParticipantCount(fields.participantCount());
        if (fields.targetRequirement() != null) post.setTargetRequirement(fields.targetRequirement());
        if (fields.contactPreference() != null) post.setContactPreference(fields.contactPreference());
        if (fields.tags() != null) post.setTags(fields.tags());
        if (fields.attachmentIds() != null) post.setAttachmentIds(fields.attachmentIds());
        if (fields.scenePayload() != null) post.setScenePayload(fields.scenePayload());
    }

    private PostResponse toResponse(PartnerPost post) {
        List<String> allowedActions = new ArrayList<>();
        if ("DRAFT".equals(post.getStatus())) {
            allowedActions.add("EDIT");
        }

        return new PostResponse(
                post.getId().toString(),
                post.getPublisherId().toString(),
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
                post.getContactPreference(),
                post.getTags(),
                post.getAttachmentIds(),
                post.getScenePayload(),
                post.getPublishedAt() != null ? post.getPublishedAt().toString() : null,
                post.getCreatedAt().toString(),
                post.getUpdatedAt().toString(),
                allowedActions
        );
    }

    public record CreateDraftRequest(
            String sceneType, String title, String description, String timeMode, String timeText,
            Instant startAt, Instant endAt, String locationText, Integer participantCount,
            String targetRequirement, String contactPreference, List<String> tags,
            List<String> attachmentIds, Map<String, Object> scenePayload
    ) implements DraftFields {}

    public record UpdateDraftRequest(
            String sceneType, String title, String description, String timeMode, String timeText,
            Instant startAt, Instant endAt, String locationText, Integer participantCount,
            String targetRequirement, String contactPreference, List<String> tags,
            List<String> attachmentIds, Map<String, Object> scenePayload
    ) implements DraftFields {}

    interface DraftFields {
        String sceneType(); String title(); String description(); String timeMode(); String timeText();
        Instant startAt(); Instant endAt(); String locationText(); Integer participantCount();
        String targetRequirement(); String contactPreference(); List<String> tags();
        List<String> attachmentIds(); Map<String, Object> scenePayload();
    }

    public record PostResponse(
            String postId, String publisherId, String sceneType, String status, String title,
            String description, String timeMode, String timeText, String startAt, String endAt,
            String locationText, Integer participantCount, String targetRequirement,
            String contactPreference, List<String> tags, List<String> attachmentIds,
            Map<String, Object> scenePayload, String publishedAt, String createdAt, String updatedAt,
            List<String> allowedActions
    ) {}

    public record PostListResponse(List<PostResponse> items, int page, int size, long totalElements, int totalPages) {}
}
