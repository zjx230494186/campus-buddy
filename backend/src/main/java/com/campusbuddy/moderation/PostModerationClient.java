package com.campusbuddy.moderation;

public interface PostModerationClient {
    PostModerationDecision moderate(PostModerationRequest request);
}
