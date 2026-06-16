package com.campusbuddy.moderation;

import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public record PostModerationRequest(
        UUID postId,
        UUID publisherId,
        String sceneType,
        String title,
        String description,
        String timeMode,
        String timeText,
        Instant startAt,
        Instant endAt,
        String locationText,
        Integer participantCount,
        String targetRequirement,
        String contactPreference,
        List<String> tags,
        Map<String, Object> scenePayload
) {}
