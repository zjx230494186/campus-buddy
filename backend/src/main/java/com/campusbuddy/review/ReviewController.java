package com.campusbuddy.review;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
@RequestMapping("/api/me/reviews")
public class ReviewController {

    private final ReviewService reviewService;
    private final CreditSummaryService creditSummaryService;

    public ReviewController(ReviewService reviewService, CreditSummaryService creditSummaryService) {
        this.reviewService = reviewService;
        this.creditSummaryService = creditSummaryService;
    }

    @PostMapping(consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ReviewService.ReviewResponse> createReview(
            Authentication authentication,
            @RequestBody ReviewService.CreateReviewRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ReviewService.ReviewResponse response = reviewService.createReview(userId, request);
        return ResponseEntity.status(201).body(response);
    }

    @PutMapping(value = "/{reviewId}", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ReviewService.ReviewResponse> updateReview(
            Authentication authentication,
            @PathVariable Long reviewId,
            @RequestBody ReviewService.UpdateReviewRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ReviewService.ReviewResponse response = reviewService.updateReview(userId, reviewId, request);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/given", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ReviewService.ReviewListResponse> listGivenReviews(
            Authentication authentication,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ReviewService.ReviewListResponse response = reviewService.listGivenReviews(userId, page, size);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/received", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ReviewService.ReviewListResponse> listReceivedReviews(
            Authentication authentication,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ReviewService.ReviewListResponse response = reviewService.listReceivedReviews(userId, page, size);
        return ResponseEntity.ok(response);
    }
}
