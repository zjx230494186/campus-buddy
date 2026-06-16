package com.campusbuddy.moderation;

import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostRepository;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.condition.EnabledIfEnvironmentVariable;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ActiveProfiles;
import org.springframework.test.context.TestPropertySource;

import java.time.Instant;
import java.util.Map;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

@SpringBootTest
@ActiveProfiles("local-h2")
@TestPropertySource(properties = {
        "campus-buddy.post-moderation.enabled=true",
        "campus-buddy.post-moderation.provider=openai-compatible",
        "campus-buddy.post-moderation.model=${POST_MODERATION_MODEL:qwen-plus}",
        "campus-buddy.post-moderation.base-url=${POST_MODERATION_BASE_URL:https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions}",
        "campus-buddy.post-moderation.api-key=${POST_MODERATION_API_KEY}",
        "campus-buddy.post-moderation.auto-approve-threshold=0.50"
})
@EnabledIfEnvironmentVariable(named = "POST_MODERATION_API_KEY", matches = ".+")
class RealQwenPostModerationSmokeTest {

    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository postRepository;
    @Autowired private PostModerationService postModerationService;

    @Test
    void qwenCanAutoApproveSafeCampusPost() {
        Instant now = Instant.now();
        UserAccount user = new UserAccount(
                "real-qwen-smoke@campus.edu.cn",
                "not-used-in-smoke",
                "RealQwenSmoke",
                now);
        user.setAuthenticationStatus("VERIFIED");
        user = userAccountRepository.save(user);

        PartnerPost post = new PartnerPost(user.getUserId(), "PENDING_REVIEW", now);
        post.setSceneType("STUDY");
        post.setTitle("Study Partner Wanted");
        post.setDescription("Looking for a study buddy to review calculus homework in the library this weekend.");
        post.setTimeMode("TEXT_PREFERENCE");
        post.setTimeText("Saturday afternoon");
        post.setLocationText("Main Library 3F");
        post.setParticipantCount(2);
        post.setTargetRequirement("Be on time and study quietly together");
        post.setContactPreference("Use in-app chat");
        post.setScenePayload(Map.of("studyGoal", "Review calculus homework"));
        post = postRepository.save(post);

        postModerationService.moderateAfterSubmit(post.getId());

        PartnerPost latest = postRepository.findById(post.getId()).orElseThrow();
        assertEquals("PUBLISHED", latest.getStatus());
        assertNotNull(latest.getPublishedAt());
        assertEquals(PostModerationService.SYSTEM_REVIEWER_ID, latest.getReviewedBy());
    }
}
