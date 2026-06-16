package com.campusbuddy.contact;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.UUID;

@RestController
public class ContactConversationController {

    private final ContactConversationService contactConversationService;
    private final ContactUnlockService contactUnlockService;

    ContactConversationController(ContactConversationService contactConversationService,
                                  ContactUnlockService contactUnlockService) {
        this.contactConversationService = contactConversationService;
        this.contactUnlockService = contactUnlockService;
    }

    @PostMapping(
            value = "/api/partner-posts/{postId}/contact-requests",
            consumes = MediaType.APPLICATION_JSON_VALUE,
            produces = MediaType.APPLICATION_JSON_VALUE
    )
    ResponseEntity<ContactConversationService.ContactRequestResponse> requestContact(
            Authentication authentication,
            @PathVariable UUID postId,
            @RequestBody ContactRequestRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ContactConversationService.ContactRequestResponse response =
                contactConversationService.requestContact(userId, postId, request.message());
        return ResponseEntity.ok(response);
    }

    @PostMapping(
            value = "/api/me/conversations/{conversationId}/messages",
            consumes = MediaType.APPLICATION_JSON_VALUE,
            produces = MediaType.APPLICATION_JSON_VALUE
    )
    ResponseEntity<ContactConversationService.SendMessageResponse> sendMessage(
            Authentication authentication,
            @PathVariable Long conversationId,
            @RequestBody SendMessageRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ContactConversationService.SendMessageResponse response =
                contactConversationService.sendMessage(userId, conversationId, request.message());
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/api/me/conversations", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactConversationService.ConversationListResponse> listConversations(
            Authentication authentication,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ContactConversationService.ConversationListResponse response =
                contactConversationService.listConversations(userId, page, size);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/api/me/conversations/{conversationId}/messages", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactConversationService.MessageListResponse> getMessages(
            Authentication authentication,
            @PathVariable Long conversationId,
            @RequestParam(required = false) Long afterMessageId,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ContactConversationService.MessageListResponse response =
                contactConversationService.getMessages(userId, conversationId, afterMessageId, page, size);
        return ResponseEntity.ok(response);
    }

    @PostMapping(value = "/api/me/conversations/{conversationId}/close", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactConversationService.CloseConversationResponse> closeConversation(
            Authentication authentication,
            @PathVariable Long conversationId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ContactConversationService.CloseConversationResponse response =
                contactConversationService.closeConversation(userId, conversationId);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/api/me/conversations/{conversationId}/can-close", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactConversationService.CanCloseResponse> canCloseConversation(
            Authentication authentication,
            @PathVariable Long conversationId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        ContactConversationService.CanCloseResponse response =
                contactConversationService.canCloseConversation(userId, conversationId);
        return ResponseEntity.ok(response);
    }

    @PostMapping(value = "/api/me/conversations/{conversationId}/read", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<Void> markConversationRead(
            Authentication authentication,
            @PathVariable Long conversationId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        contactConversationService.markConversationRead(userId, conversationId);
        return ResponseEntity.ok().build();
    }

    public record ContactRequestRequest(String message) {}

    public record SendMessageRequest(String message) {}

    @GetMapping(value = "/api/me/conversations/{conversationId}/contact-unlock", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactUnlockService.ContactUnlockStatusResponse> getContactUnlockStatus(
            Authentication authentication,
            @PathVariable Long conversationId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        return ResponseEntity.ok(contactUnlockService.getUnlockStatus(userId, conversationId));
    }

    @PostMapping(value = "/api/me/conversations/{conversationId}/contact-unlock/confirm", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactUnlockService.ContactUnlockStatusResponse> confirmContactUnlock(
            Authentication authentication,
            @PathVariable Long conversationId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        return ResponseEntity.ok(contactUnlockService.confirmUnlock(userId, conversationId));
    }

    @GetMapping(value = "/api/me/conversations/{conversationId}/peer-contact-card", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<ContactUnlockService.PeerContactCardResponse> getPeerContactCard(
            Authentication authentication,
            @PathVariable Long conversationId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        return ResponseEntity.ok(contactUnlockService.getPeerContactCard(userId, conversationId));
    }
}
