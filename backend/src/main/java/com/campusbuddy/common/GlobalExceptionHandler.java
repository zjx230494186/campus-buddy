package com.campusbuddy.common;

import jakarta.servlet.http.HttpServletRequest;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.servlet.resource.NoResourceFoundException;

@RestControllerAdvice
public class GlobalExceptionHandler {

    @ExceptionHandler(NoResourceFoundException.class)
    public ResponseEntity<ApiErrorResponse> handleNoResourceFound(
            NoResourceFoundException exception,
            HttpServletRequest request
    ) {
        ApiErrorResponse response = new ApiErrorResponse(
                "RESOURCE_NOT_FOUND",
                "Resource not found",
                request.getRequestURI(),
                traceId(request)
        );

        return ResponseEntity.status(HttpStatus.NOT_FOUND).body(response);
    }

    @ExceptionHandler(ApiException.class)
    public ResponseEntity<ApiErrorResponse> handleApiException(
            ApiException exception,
            HttpServletRequest request
    ) {
        ApiErrorResponse response = new ApiErrorResponse(
                exception.code(),
                exception.getMessage(),
                exception.details(),
                traceId(request)
        );

        return ResponseEntity.status(exception.status()).body(response);
    }

    private String traceId(HttpServletRequest request) {
        Object traceId = request.getAttribute(TraceIdFilter.TRACE_ID_ATTRIBUTE);
        if (traceId instanceof String value && !value.isBlank()) {
            return value;
        }
        return "missing-trace-id";
    }
}
