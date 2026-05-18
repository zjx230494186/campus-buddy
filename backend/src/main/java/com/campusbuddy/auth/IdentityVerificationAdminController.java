package com.campusbuddy.auth;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.access.prepost.PreAuthorize;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;
import java.util.UUID;

@RestController
@RequestMapping("/api/admin/identity-verifications")
public class IdentityVerificationAdminController {

    private final IdentityVerificationAdminService adminService;

    IdentityVerificationAdminController(IdentityVerificationAdminService adminService) {
        this.adminService = adminService;
    }

    @GetMapping(produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<List<IdentityVerificationAdminService.PendingSubmissionItem>> listPending(
            @RequestParam(value = "status", defaultValue = "PENDING_REVIEW") String status
    ) {
        if (!"PENDING_REVIEW".equals(status)) {
            return ResponseEntity.ok(List.of());
        }
        return ResponseEntity.ok(adminService.listPending());
    }

    @PostMapping(value = "/{submissionId}/reviews", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<IdentityVerificationAdminService.ReviewResponse> review(
            @PathVariable UUID submissionId,
            @RequestBody IdentityVerificationAdminService.ReviewRequest request
    ) {
        IdentityVerificationAdminService.ReviewResponse response = adminService.review(submissionId, request);
        return ResponseEntity.ok(response);
    }
}
