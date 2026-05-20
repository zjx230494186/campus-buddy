package com.campusbuddy.review;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
@RequestMapping("/api")
public class CreditSummaryController {

    private final CreditSummaryService creditSummaryService;

    public CreditSummaryController(CreditSummaryService creditSummaryService) {
        this.creditSummaryService = creditSummaryService;
    }

    @GetMapping(value = "/users/{userId}/credit-summary", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<CreditSummaryService.PublicCreditSummaryResponse> getPublicCreditSummary(
            @PathVariable UUID userId
    ) {
        CreditSummaryService.CreditSummaryResponse summary = creditSummaryService.getCreditSummary(userId, false);
        return ResponseEntity.ok(creditSummaryService.toPublicResponse(summary));
    }

    @GetMapping(value = "/me/credit-summary", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<CreditSummaryService.CreditSummaryResponse> getMyCreditSummary(
            Authentication authentication
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        CreditSummaryService.CreditSummaryResponse response = creditSummaryService.getCreditSummary(userId, true);
        return ResponseEntity.ok(response);
    }
}
