package com.campusbuddy.moderation;

public record PostModerationDecision(
        String decision,
        double confidence,
        String reason,
        String userVisibleReason,
        String category
) {}
