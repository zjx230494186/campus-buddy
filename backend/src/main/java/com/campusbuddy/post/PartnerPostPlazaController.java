package com.campusbuddy.post;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
@RequestMapping("/api/partner-posts")
public class PartnerPostPlazaController {

    private final PartnerPostPlazaService plazaService;

    PartnerPostPlazaController(PartnerPostPlazaService plazaService) {
        this.plazaService = plazaService;
    }

    @GetMapping(produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostPlazaService.PlazaListResponse> listPosts(
            Authentication authentication,
            @RequestParam(required = false) String sceneType,
            @RequestParam(required = false) String keyword,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        PartnerPostPlazaService.PlazaListResponse response = plazaService.listPosts(userId, sceneType, keyword, page, size);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/{postId}", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<PartnerPostPlazaService.PlazaDetailResponse> getPostDetail(
            Authentication authentication,
            @PathVariable UUID postId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        PartnerPostPlazaService.PlazaDetailResponse response = plazaService.getPostDetail(userId, postId);
        return ResponseEntity.ok(response);
    }
}
