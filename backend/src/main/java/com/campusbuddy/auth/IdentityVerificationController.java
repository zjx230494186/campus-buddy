package com.campusbuddy.auth;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.UUID;

@RestController
@RequestMapping("/api/auth/identity-verifications")
public class IdentityVerificationController {

    private final IdentityVerificationService identityVerificationService;

    IdentityVerificationController(IdentityVerificationService identityVerificationService) {
        this.identityVerificationService = identityVerificationService;
    }

    @PostMapping(consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<IdentityVerificationService.IdentityVerificationSubmitResponse> submit(
            Authentication authentication,
            @RequestBody IdentityVerificationService.IdentityVerificationSubmitRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        IdentityVerificationService.IdentityVerificationSubmitResponse response = identityVerificationService.submit(userId, request);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/me", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<IdentityVerificationService.IdentityVerificationStatusResponse> getStatus(Authentication authentication) {
        UUID userId = UUID.fromString(authentication.getName());
        IdentityVerificationService.IdentityVerificationStatusResponse response = identityVerificationService.getStatus(userId);
        return ResponseEntity.ok(response);
    }
}
