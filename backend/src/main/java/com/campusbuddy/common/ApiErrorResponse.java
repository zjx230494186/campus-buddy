package com.campusbuddy.common;

public record ApiErrorResponse(
        String code,
        String message,
        Object details,
        String traceId
) {
}
