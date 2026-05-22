package com.campusbuddy.post;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
@RequestMapping("/api/me/partner-posts")
public class PartnerPostController {

    private final PartnerPostService postService;

    PartnerPostController(PartnerPostService postService) {
        this.postService = postService;
    }

    @PostMapping(consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostService.PostResponse> createDraft(
            Authentication authentication,
            @RequestBody PartnerPostService.CreateDraftRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        PartnerPostService.PostResponse response = postService.createDraft(userId, request);
        return ResponseEntity.ok(response);
    }

    @PutMapping(value = "/{postId}", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostService.PostResponse> updateDraft(
            Authentication authentication,
            @PathVariable UUID postId,
            @RequestBody PartnerPostService.UpdateDraftRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        PartnerPostService.PostResponse response = postService.updateDraft(userId, postId, request);
        return ResponseEntity.ok(response);
    }

    @GetMapping(produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostService.PostListResponse> listMyPosts(
            Authentication authentication,
            @RequestParam(required = false) String status,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        PartnerPostService.PostListResponse response = postService.listMyPosts(userId, status, page, size);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/{postId}", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostService.PostResponse> getMyPost(
            Authentication authentication,
            @PathVariable UUID postId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        PartnerPostService.PostResponse response = postService.getMyPost(userId, postId);
        return ResponseEntity.ok(response);
    }
}
