package com.campusbuddy.post;

import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import com.campusbuddy.review.CreditSummaryService;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.*;

@Service
public class PartnerPostPlazaService {

    private static final int MAX_PAGE_SIZE = 50;

    private final PartnerPostRepository postRepository;
    private final UserAccountRepository userAccountRepository;
    private final CreditSummaryService creditSummaryService;

    PartnerPostPlazaService(PartnerPostRepository postRepository, UserAccountRepository userAccountRepository, CreditSummaryService creditSummaryService) {
        this.postRepository = postRepository;
        this.userAccountRepository = userAccountRepository;
        this.creditSummaryService = creditSummaryService;
    }

    public PlazaListResponse listPosts(UUID currentUserId, String sceneType, String keyword, int page, int size) {
        int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
        PageRequest pageable = PageRequest.of(page, safeSize);

        Page<PartnerPost> postPage;
        boolean hasSceneType = sceneType != null && !sceneType.isBlank();
        boolean hasKeyword = keyword != null && !keyword.isBlank();

        if (hasSceneType && hasKeyword) {
            postPage = postRepository.searchPublishedBySceneTypeAndKeyword(sceneType, keyword, pageable);
        } else if (hasKeyword) {
            postPage = postRepository.searchPublishedByKeyword(keyword, pageable);
        } else if (hasSceneType) {
            postPage = postRepository.findPublishedBySceneType(sceneType, pageable);
        } else {
            postPage = postRepository.findAllPublished(pageable);
        }

        List<PlazaListItem> items = postPage.getContent().stream()
                .map(p -> toListItem(p, currentUserId))
                .toList();

        return new PlazaListResponse(items, postPage.getNumber(), postPage.getSize(), postPage.getTotalElements(), postPage.getTotalPages());
    }

    @Transactional(readOnly = true)
    public PlazaDetailResponse getPostDetail(UUID currentUserId, UUID postId) {
        PartnerPost post = postRepository.findByIdAndStatus(postId, "PUBLISHED")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist or is not published"));

        UserAccount publisher = userAccountRepository.findById(post.getPublisherId()).orElse(null);
        String publisherDisplayName = publisher != null ? publisher.getDisplayName() : null;
        String publisherAuthenticationStatus = publisher != null ? publisher.getAuthenticationStatus() : null;

        CreditSummaryService.PublicCreditSummaryResponse creditSummary = null;
        if (publisher != null) {
            try {
                CreditSummaryService.CreditSummaryResponse fullSummary = creditSummaryService.getCreditSummary(publisher.getUserId(), false);
                creditSummary = creditSummaryService.toPublicResponse(fullSummary);
            } catch (Exception ignored) {
            }
        }

        boolean ownPost = post.getPublisherId().equals(currentUserId);

        return new PlazaDetailResponse(
                post.getId().toString(),
                post.getPublisherId().toString(),
                publisherDisplayName,
                publisherAuthenticationStatus,
                creditSummary,
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
                post.getPublishedAt() != null ? post.getPublishedAt().toString() : null,
                post.getUpdatedAt().toString(),
                ownPost
        );
    }

    private PlazaListItem toListItem(PartnerPost post, UUID currentUserId) {
        UserAccount publisher = userAccountRepository.findById(post.getPublisherId()).orElse(null);
        String publisherDisplayName = publisher != null ? publisher.getDisplayName() : null;
        String publisherAuthenticationStatus = publisher != null ? publisher.getAuthenticationStatus() : null;
        boolean ownPost = post.getPublisherId().equals(currentUserId);

        CreditSummaryService.PublicCreditSummaryResponse creditSummary = null;
        if (publisher != null) {
            try {
                CreditSummaryService.CreditSummaryResponse fullSummary = creditSummaryService.getCreditSummary(publisher.getUserId(), false);
                creditSummary = creditSummaryService.toPublicResponse(fullSummary);
            } catch (Exception ignored) {
            }
        }

        return new PlazaListItem(
                post.getId().toString(),
                post.getPublisherId().toString(),
                publisherDisplayName,
                publisherAuthenticationStatus,
                creditSummary,
                post.getSceneType(),
                post.getStatus(),
                post.getTitle(),
                post.getDescription(),
                post.getTags(),
                post.getTimeText(),
                post.getLocationText(),
                post.getScenePayload(),
                post.getPublishedAt() != null ? post.getPublishedAt().toString() : null,
                post.getUpdatedAt().toString(),
                ownPost
        );
    }

    public record PlazaListItem(
            String postId, String publisherId, String publisherDisplayName,
            String publisherAuthenticationStatus,
            CreditSummaryService.PublicCreditSummaryResponse publisherCreditSummary,
            String sceneType, String status, String title, String description,
            List<String> tags, String timeText, String locationText,
            Map<String, Object> scenePayload, String publishedAt, String updatedAt,
            boolean ownPost
    ) {}

    public record PlazaDetailResponse(
            String postId, String publisherId, String publisherDisplayName,
            String publisherAuthenticationStatus,
            CreditSummaryService.PublicCreditSummaryResponse publisherCreditSummary,
            String sceneType, String status, String title, String description,
            String timeMode, String timeText, String startAt, String endAt,
            String locationText, Integer participantCount, String targetRequirement,
            List<String> tags, Map<String, Object> scenePayload,
            String publishedAt, String updatedAt, boolean ownPost
    ) {}

    public record PlazaListResponse(
            List<PlazaListItem> items, int page, int size, long totalElements, int totalPages
    ) {}
}
