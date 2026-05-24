package com.campusbuddy.contact;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
public class ContactCardController {

    private final ContactCardService contactCardService;

    ContactCardController(ContactCardService contactCardService) {
        this.contactCardService = contactCardService;
    }

    @GetMapping(value = "/api/me/contact-card", produces = MediaType.APPLICATION_JSON_VALUE)
    public ResponseEntity<ContactCardService.ContactCardResponse> getContactCard(Authentication authentication) {
        UUID userId = UUID.fromString(authentication.getName());
        return ResponseEntity.ok(contactCardService.getContactCard(userId));
    }

    @PutMapping(value = "/api/me/contact-card", consumes = MediaType.APPLICATION_JSON_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    public ResponseEntity<ContactCardService.ContactCardResponse> upsertContactCard(
            Authentication authentication, @RequestBody UpsertContactCardRequest request) {
        UUID userId = UUID.fromString(authentication.getName());
        return ResponseEntity.ok(contactCardService.upsertContactCard(userId, request.wechatId(), request.phoneNumber(), request.qqNumber()));
    }

    public record UpsertContactCardRequest(String wechatId, String phoneNumber, String qqNumber) {}
}
