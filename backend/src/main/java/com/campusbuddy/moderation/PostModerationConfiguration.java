package com.campusbuddy.moderation;

import org.springframework.boot.autoconfigure.condition.ConditionalOnExpression;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

@Configuration
class PostModerationConfiguration {

    @Bean
    @ConditionalOnExpression("'${campus-buddy.post-moderation.provider:noop}' == 'openai' || '${campus-buddy.post-moderation.provider:noop}' == 'openai-compatible'")
    PostModerationClient openAiCompatiblePostModerationClient(PostModerationProperties properties) {
        return new OpenAiCompatiblePostModerationClient(properties);
    }
}
