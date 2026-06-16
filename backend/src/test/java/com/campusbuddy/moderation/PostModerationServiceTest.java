package com.campusbuddy.moderation;

import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostAdminService;
import com.campusbuddy.post.PartnerPostRepository;
import org.junit.jupiter.api.Test;
import org.springframework.test.util.ReflectionTestUtils;

import java.time.Instant;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNull;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

class PostModerationServiceTest {

    private final PartnerPostRepository postRepository = mock(PartnerPostRepository.class);
    private final PartnerPostAdminService adminService = mock(PartnerPostAdminService.class);
    private final PostModerationClient client = mock(PostModerationClient.class);
    private final PostModerationProperties properties = new PostModerationProperties();
    private final PostModerationService service = new PostModerationService(postRepository, adminService, client, properties);

    @Test
    void disabledModerationKeepsPendingReviewWithoutCallingClient() {
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));

        service.moderateAfterSubmit(post.getId());

        assertEquals("PENDING_REVIEW", post.getStatus());
        verifyNoInteractions(client);
        verifyNoInteractions(adminService);
    }

    @Test
    void needsHumanDecisionKeepsPendingReview() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenReturn(new PostModerationDecision(
                "NEEDS_HUMAN", 0.70, "context is unclear", "请人工审核", null));

        service.moderateAfterSubmit(post.getId());

        verify(client).moderate(any());
        verifyNoInteractions(adminService);
    }

    @Test
    void highConfidenceApproveDelegatesToAdminApprovalFlow() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenReturn(new PostModerationDecision(
                "APPROVE", 0.95, "safe campus post", null, null));

        service.moderateAfterSubmit(post.getId());

        verify(adminService).approvePendingPost(eq(PostModerationService.SYSTEM_REVIEWER_ID), eq(post.getId()), any(Instant.class));
        verify(adminService, never()).rejectPendingPost(any(), any(), anyString(), any());
    }

    @Test
    void highConfidenceRejectDelegatesToAdminRejectFlow() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenReturn(new PostModerationDecision(
                "REJECT", 0.91, "contains unsafe content", "内容不符合平台发布规范，请修改后重新提交。", "SAFETY"));

        service.moderateAfterSubmit(post.getId());

        verify(adminService).rejectPendingPost(
                eq(PostModerationService.SYSTEM_REVIEWER_ID),
                eq(post.getId()),
                eq("内容不符合平台发布规范，请修改后重新提交。"),
                any(Instant.class));
        verify(adminService, never()).approvePendingPost(any(), any(), any());
    }

    @Test
    void lowConfidenceRejectKeepsPendingReview() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenReturn(new PostModerationDecision(
                "REJECT", 0.60, "not enough confidence", "请修改内容", "OTHER"));

        service.moderateAfterSubmit(post.getId());

        verifyNoInteractions(adminService);
    }

    @Test
    void rejectWithoutVisibleReasonKeepsPendingReview() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenReturn(new PostModerationDecision(
                "REJECT", 0.96, "missing user reason", " ", "OTHER"));

        service.moderateAfterSubmit(post.getId());

        verifyNoInteractions(adminService);
    }

    @Test
    void clientFailureKeepsPendingReview() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenThrow(new IllegalStateException("provider failed"));

        service.moderateAfterSubmit(post.getId());

        verifyNoInteractions(adminService);
    }

    @Test
    void buildsModerationRequestWithoutDirectContactPreference() {
        properties.setEnabled(true);
        PartnerPost post = pendingPost();
        when(postRepository.findById(post.getId())).thenReturn(Optional.of(post));
        when(client.moderate(any())).thenReturn(new PostModerationDecision(
                "NEEDS_HUMAN", 0.50, "manual", null, null));

        service.moderateAfterSubmit(post.getId());

        verify(client).moderate(argThat(request ->
                request.postId().equals(post.getId())
                        && request.publisherId().equals(post.getPublisherId())
                        && request.contactPreference() == null
                        && request.title().equals(post.getTitle())
                        && request.scenePayload().containsKey("studyGoal")));
    }

    private PartnerPost pendingPost() {
        PartnerPost post = new PartnerPost(UUID.randomUUID(), "PENDING_REVIEW", Instant.parse("2026-06-16T00:00:00Z"));
        ReflectionTestUtils.setField(post, "id", UUID.randomUUID());
        post.setSceneType("STUDY");
        post.setTitle("Study Partner Wanted");
        post.setDescription("Looking for a study buddy for finals");
        post.setTimeMode("TEXT_PREFERENCE");
        post.setTimeText("Weekends");
        post.setLocationText("Main Library");
        post.setParticipantCount(2);
        post.setTargetRequirement("GPA above 3.0");
        post.setContactPreference("WeChat campus_buddy");
        post.setScenePayload(Map.of("studyGoal", "Pass final exam"));
        return post;
    }
}
