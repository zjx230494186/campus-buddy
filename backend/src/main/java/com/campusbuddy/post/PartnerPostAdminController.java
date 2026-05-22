package com.campusbuddy.post;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
@RequestMapping("/api/admin/partner-posts")
public class PartnerPostAdminController {

    private final PartnerPostAdminService adminService;

    PartnerPostAdminController(PartnerPostAdminService adminService) {
        this.adminService = adminService;
    }

    @GetMapping(value = "/review-queue", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostAdminService.ReviewQueueResponse> reviewQueue(
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        PartnerPostAdminService.ReviewQueueResponse response = adminService.reviewQueue(page, size);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/{postId}", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostAdminService.AdminPostDetailResponse> getPostDetail(
            @PathVariable UUID postId
    ) {
        PartnerPostAdminService.AdminPostDetailResponse response = adminService.getPostDetail(postId);
        return ResponseEntity.ok(response);
    }

    @PostMapping(value = "/{postId}/review", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostAdminService.AdminPostDetailResponse> reviewPost(
            Authentication authentication,
            @PathVariable UUID postId,
            @RequestBody PartnerPostAdminService.ReviewDecisionRequest request
    ) {
        UUID adminId = UUID.fromString(authentication.getName());
        PartnerPostAdminService.AdminPostDetailResponse response = adminService.reviewPost(adminId, postId, request);
        return ResponseEntity.ok(response);
    }
}
