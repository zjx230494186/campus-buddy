package com.campusbuddy.contact;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.UUID;

@RestController
@RequestMapping("/api/me")
public class GroupChatController {

    private final GroupChatService groupChatService;

    GroupChatController(GroupChatService groupChatService) {
        this.groupChatService = groupChatService;
    }

    @PostMapping(
            value = "/group-chats",
            consumes = MediaType.APPLICATION_JSON_VALUE,
            produces = MediaType.APPLICATION_JSON_VALUE
    )
    ResponseEntity<GroupChatService.CreateGroupChatResponse> createGroupChat(
            Authentication authentication,
            @RequestBody CreateGroupChatRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.CreateGroupChatResponse response = groupChatService.createGroupChat(userId, request.toServiceRequest());
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/group-chats", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<GroupChatService.GroupChatListResponse> listGroupChats(
            Authentication authentication,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.GroupChatListResponse response = groupChatService.listGroupChats(userId, page, size);
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/group-chats/{groupChatId}", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<GroupChatService.GroupChatDetailResponse> getGroupChatDetail(
            Authentication authentication,
            @PathVariable Long groupChatId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.GroupChatDetailResponse response = groupChatService.getGroupChatDetail(userId, groupChatId);
        return ResponseEntity.ok(response);
    }

    @PutMapping(
            value = "/group-chats/{groupChatId}",
            consumes = MediaType.APPLICATION_JSON_VALUE,
            produces = MediaType.APPLICATION_JSON_VALUE
    )
    ResponseEntity<GroupChatService.UpdateGroupChatResponse> updateGroupChat(
            Authentication authentication,
            @PathVariable Long groupChatId,
            @RequestBody UpdateGroupChatRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.UpdateGroupChatResponse response = groupChatService.updateGroupChat(userId, groupChatId, request.toServiceRequest());
        return ResponseEntity.ok(response);
    }

    @DeleteMapping(value = "/group-chats/{groupChatId}", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<GroupChatService.CloseGroupChatResponse> closeGroupChat(
            Authentication authentication,
            @PathVariable Long groupChatId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.CloseGroupChatResponse response = groupChatService.closeGroupChat(userId, groupChatId);
        return ResponseEntity.ok(response);
    }

    @PostMapping(
            value = "/group-chats/{groupChatId}/messages",
            consumes = MediaType.APPLICATION_JSON_VALUE,
            produces = MediaType.APPLICATION_JSON_VALUE
    )
    ResponseEntity<GroupChatService.SendGroupMessageResponse> sendMessage(
            Authentication authentication,
            @PathVariable Long groupChatId,
            @RequestBody SendMessageRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.SendGroupMessageResponse response = groupChatService.sendMessage(userId, groupChatId, request.message());
        return ResponseEntity.ok(response);
    }

    @GetMapping(value = "/group-chats/{groupChatId}/messages", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<GroupChatService.GroupMessageListResponse> getMessages(
            Authentication authentication,
            @PathVariable Long groupChatId,
            @RequestParam(required = false) Long afterMessageId,
            @RequestParam(defaultValue = "0") int page,
            @RequestParam(defaultValue = "20") int size
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.GroupMessageListResponse response = groupChatService.getMessages(userId, groupChatId, afterMessageId, page, size);
        return ResponseEntity.ok(response);
    }

    @PostMapping(value = "/group-chats/{groupChatId}/read", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<Void> markGroupChatRead(
            Authentication authentication,
            @PathVariable Long groupChatId
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        groupChatService.markGroupChatRead(userId, groupChatId);
        return ResponseEntity.ok().build();
    }

    @PostMapping(
            value = "/group-chats/{groupChatId}/members",
            consumes = MediaType.APPLICATION_JSON_VALUE,
            produces = MediaType.APPLICATION_JSON_VALUE
    )
    ResponseEntity<GroupChatService.AddMembersResponse> addMembers(
            Authentication authentication,
            @PathVariable Long groupChatId,
            @RequestBody AddMembersRequest request
    ) {
        UUID userId = UUID.fromString(authentication.getName());
        GroupChatService.AddMembersResponse response = groupChatService.addMembers(userId, groupChatId, request.toUuidList());
        return ResponseEntity.ok(response);
    }

    @DeleteMapping(value = "/group-chats/{groupChatId}/members/{userId}", produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<GroupChatService.RemoveMemberResponse> removeMember(
            Authentication authentication,
            @PathVariable Long groupChatId,
            @PathVariable UUID userId
    ) {
        UUID currentUserId = UUID.fromString(authentication.getName());
        GroupChatService.RemoveMemberResponse response = groupChatService.removeMember(currentUserId, groupChatId, userId);
        return ResponseEntity.ok(response);
    }

    public record CreateGroupChatRequest(String name, String description, String relatedPostUuid,
                                         Integer maxMembers, List<String> initialMemberIds) {
        public GroupChatService.CreateGroupChatRequest toServiceRequest() {
            UUID relatedPostUuidValue = null;
            if (relatedPostUuid != null && !relatedPostUuid.isBlank()) {
                try {
                    relatedPostUuidValue = UUID.fromString(relatedPostUuid);
                } catch (IllegalArgumentException e) {
                    // Invalid UUID format, treat as null
                }
            }
            List<UUID> initialMemberUuids = null;
            if (initialMemberIds != null && !initialMemberIds.isEmpty()) {
                initialMemberUuids = initialMemberIds.stream()
                    .filter(id -> id != null && !id.isBlank())
                    .map(UUID::fromString)
                    .toList();
            }
            return new GroupChatService.CreateGroupChatRequest(name, description, relatedPostUuidValue, maxMembers, initialMemberUuids);
        }
    }

    public record UpdateGroupChatRequest(String name, String description, Integer maxMembers) {
        public GroupChatService.UpdateGroupChatRequest toServiceRequest() {
            return new GroupChatService.UpdateGroupChatRequest(name, description, maxMembers);
        }
    }

    public record SendMessageRequest(String message) {}

    public record AddMembersRequest(List<String> userIds) {
        public List<UUID> toUuidList() {
            if (userIds == null || userIds.isEmpty()) return List.of();
            return userIds.stream()
                .filter(id -> id != null && !id.isBlank())
                .map(UUID::fromString)
                .toList();
        }
    }
}