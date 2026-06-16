package com.campusbuddy.moderation;

import org.springframework.boot.autoconfigure.condition.ConditionalOnMissingBean;
import org.springframework.stereotype.Component;

@Component
@ConditionalOnMissingBean(PostModerationClient.class)
class NoopPostModerationClient implements PostModerationClient {

    @Override
    public PostModerationDecision moderate(PostModerationRequest request) {
        return new PostModerationDecision("NEEDS_HUMAN", 0.0, "post moderation provider is noop", null, null);
    }
}
