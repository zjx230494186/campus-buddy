package com.campusbuddy.contact;

import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostRepository;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.*;
import java.util.stream.Collectors;

@Service
public class GroupChatService {

    private static final int MAX_MESSAGE_LENGTH = 500;
    private static final int MAX_PAGE_SIZE = 50;
    private static final int DEFAULT_MAX_MEMBERS = 20;
    private static final int MIN_MAX_MEMBERS = 2;
    private static final int MAX_MAX_MEMBERS = 100;

    private final GroupChatRepository groupChatRepository;
    private final GroupChatMemberRepository groupChatMemberRepository;
    private final GroupChatMessageRepository groupChatMessageRepository;
    private final PartnerPostRepository partnerPostRepository;
    private final UserAccountRepository userAccountRepository;

    GroupChatService(GroupChatRepository groupChatRepository,
                     GroupChatMemberRepository groupChatMemberRepository,
                     GroupChatMessageRepository groupChatMessageRepository,
                     PartnerPostRepository partnerPostRepository,
                     UserAccountRepository userAccountRepository) {
        this.groupChatRepository = groupChatRepository;
        this.groupChatMemberRepository = groupChatMemberRepository;
        this.groupChatMessageRepository = groupChatMessageRepository;
        this.partnerPostRepository = partnerPostRepository;
        this.userAccountRepository = userAccountRepository;
    }

    @Transactional
    public CreateGroupChatResponse createGroupChat(UUID currentUserId, CreateGroupChatRequest request) {
        validateUser(currentUserId);

        int maxMembers = request.maxMembers() != null ? request.maxMembers() : DEFAULT_MAX_MEMBERS;
        validateMaxMembers(maxMembers);

        GroupChat groupChat = new GroupChat(
                request.name(),
                request.description(),
                currentUserId,
                request.relatedPostUuid(),
                Instant.now()
        );
        groupChat.setMaxMembers(maxMembers);
        GroupChat savedChat = groupChatRepository.save(groupChat);

        GroupChatMember creatorMember = new GroupChatMember(savedChat.getId(), currentUserId, "ADMIN", Instant.now());
        groupChatMemberRepository.save(creatorMember);

        if (request.initialMemberIds() != null && !request.initialMemberIds().isEmpty()) {
            addMembers(currentUserId, savedChat.getId(), request.initialMemberIds());
        }

        return new CreateGroupChatResponse(savedChat.getId(), savedChat.getName(), savedChat.getStatus());
    }

    @Transactional(readOnly = true)
    public GroupChatDetailResponse getGroupChatDetail(UUID currentUserId, Long groupChatId) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureMember(currentUserId, groupChatId);

        List<GroupChatMember> members = groupChatMemberRepository.findByGroupChatIdAndStatus(groupChatId, "JOINED");
        List<MemberItem> memberItems = members.stream()
                .map(m -> toMemberItem(m))
                .toList();

        GroupChatMember currentMember = groupChatMemberRepository.findActiveMember(groupChatId, currentUserId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "NOT_MEMBER",
                        "You are not a member of this group chat", null));

        String lastMessagePreview = null;
        String lastMessageAt = null;
        List<GroupChatMessage> lastMsgs = groupChatMessageRepository.findTop1ByGroupChatIdOrderByCreatedAtDesc(groupChatId);
        if (!lastMsgs.isEmpty()) {
            GroupChatMessage lastMsg = lastMsgs.get(0);
            if (lastMsg.getContent() != null) {
                lastMessagePreview = lastMsg.getContent().length() > 30
                        ? lastMsg.getContent().substring(0, 30) + "..."
                        : lastMsg.getContent();
            }
            lastMessageAt = lastMsg.getCreatedAt().toString();
        }

        long unreadCount = computeUnreadCount(groupChatId, currentMember);

        return new GroupChatDetailResponse(
                groupChat.getId(),
                groupChat.getName(),
                groupChat.getDescription(),
                groupChat.getStatus(),
                groupChat.getMaxMembers(),
                memberItems.size(),
                currentMember.getRole(),
                lastMessagePreview,
                lastMessageAt,
                groupChat.getCreatedAt().toString(),
                groupChat.getUpdatedAt().toString(),
                memberItems,
                unreadCount
        );
    }

    @Transactional(readOnly = true)
    public GroupChatListResponse listGroupChats(UUID currentUserId, int page, int size) {
        int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
        PageRequest pageable = PageRequest.of(page, safeSize);

        Page<GroupChat> chatPage = groupChatRepository.findByMemberUserIdOrderByUpdatedAtDesc(currentUserId, pageable);

        List<GroupChatListItem> items = chatPage.getContent().stream()
                .map(chat -> toListItem(chat, currentUserId))
                .toList();

        return new GroupChatListResponse(items, chatPage.getNumber(), chatPage.getSize(),
                chatPage.getTotalElements(), chatPage.getTotalPages());
    }

    @Transactional
    public SendGroupMessageResponse sendMessage(UUID currentUserId, Long groupChatId, String message) {
        validateMessage(message);

        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureMember(currentUserId, groupChatId);

        GroupChatMessage msg = new GroupChatMessage(
                groupChatId, currentUserId, "USER_TEXT", message.trim(), Instant.now());
        GroupChatMessage saved = groupChatMessageRepository.save(msg);

        groupChat.setUpdatedAt(Instant.now());
        groupChat.setLastMessageAt(Instant.now());
        groupChatRepository.save(groupChat);

        return new SendGroupMessageResponse(saved.getId(), groupChatId);
    }

    @Transactional(readOnly = true)
    public GroupMessageListResponse getMessages(UUID currentUserId, Long groupChatId,
                                                Long afterMessageId, int page, int size) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureMember(currentUserId, groupChatId);

        if (afterMessageId != null && afterMessageId > 0) {
            int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
            PageRequest pageable = PageRequest.of(0, safeSize);
            Page<GroupChatMessage> msgPage = groupChatMessageRepository
                    .findByGroupChatIdAndIdGreaterThanOrderByIdAsc(groupChatId, afterMessageId, pageable);
            List<GroupMessageItem> msgItems = msgPage.getContent().stream().map(this::toMessageItem).toList();
            return new GroupMessageListResponse(msgItems, msgPage.getNumber(), msgPage.getSize(),
                    msgPage.getTotalElements(), msgPage.getTotalPages());
        }

        int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
        PageRequest pageable = PageRequest.of(page, safeSize);
        Page<GroupChatMessage> msgPage = groupChatMessageRepository
                .findByGroupChatIdOrderByCreatedAtAsc(groupChatId, pageable);

        List<GroupMessageItem> msgItems = msgPage.getContent().stream().map(this::toMessageItem).toList();
        return new GroupMessageListResponse(msgItems, msgPage.getNumber(), msgPage.getSize(),
                msgPage.getTotalElements(), msgPage.getTotalPages());
    }

    @Transactional
    public void markGroupChatRead(UUID currentUserId, Long groupChatId) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureMember(currentUserId, groupChatId);

        GroupChatMember member = groupChatMemberRepository.findActiveMember(groupChatId, currentUserId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "NOT_MEMBER",
                        "You are not a member of this group chat", null));

        List<GroupChatMessage> lastMsgs = groupChatMessageRepository.findTop1ByGroupChatIdOrderByCreatedAtDesc(groupChatId);
        if (!lastMsgs.isEmpty()) {
            member.setLastReadMessageId(lastMsgs.get(0).getId());
            groupChatMemberRepository.save(member);
        }
    }

    @Transactional
    public AddMembersResponse addMembers(UUID currentUserId, Long groupChatId, List<UUID> userIds) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureAdmin(currentUserId, groupChatId);

        long currentMemberCount = groupChatMemberRepository.countByGroupChatIdAndStatus(groupChatId, "JOINED");
        if (currentMemberCount + userIds.size() > groupChat.getMaxMembers()) {
            throw new ApiException(HttpStatus.FORBIDDEN, "MAX_MEMBERS_EXCEEDED",
                    "Adding these members would exceed the maximum group size", null);
        }

        List<String> added = new ArrayList<>();
        List<String> alreadyMember = new ArrayList<>();
        List<String> invalidUsers = new ArrayList<>();

        for (UUID userId : userIds) {
            if (userId.equals(currentUserId)) continue;

            UserAccount targetUser = userAccountRepository.findById(userId).orElse(null);
            if (targetUser == null) {
                invalidUsers.add(userId.toString());
                continue;
            }

            if (!"VERIFIED".equals(targetUser.getAuthenticationStatus())) {
                invalidUsers.add(userId.toString());
                continue;
            }

            Optional<GroupChatMember> existing = groupChatMemberRepository.findByGroupChatIdAndUserId(groupChatId, userId);
            if (existing.isPresent()) {
                if ("JOINED".equals(existing.get().getStatus())) {
                    alreadyMember.add(userId.toString());
                } else {
                    GroupChatMember member = existing.get();
                    member.setStatus("JOINED");
                    member.setLeftAt(null);
                    groupChatMemberRepository.save(member);
                    added.add(userId.toString());
                }
            } else {
                GroupChatMember member = new GroupChatMember(groupChatId, userId, "MEMBER", Instant.now());
                groupChatMemberRepository.save(member);
                added.add(userId.toString());
            }
        }

        groupChat.setUpdatedAt(Instant.now());
        groupChatRepository.save(groupChat);

        return new AddMembersResponse(added, alreadyMember, invalidUsers);
    }

    @Transactional
    public RemoveMemberResponse removeMember(UUID currentUserId, Long groupChatId, UUID targetUserId) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        if (!currentUserId.equals(targetUserId)) {
            ensureAdmin(currentUserId, groupChatId);
        }

        GroupChatMember member = groupChatMemberRepository.findActiveMember(groupChatId, targetUserId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "MEMBER_NOT_FOUND",
                        "Member not found in this group chat", null));

        List<GroupChatMember> admins = groupChatMemberRepository.findAdminsByGroupChatId(groupChatId);
        if (member.isAdmin() && admins.size() == 1) {
            throw new ApiException(HttpStatus.FORBIDDEN, "LAST_ADMIN_CANNOT_LEAVE",
                    "Cannot remove the last admin from the group", null);
        }

        member.setStatus("LEFT");
        member.setLeftAt(Instant.now());
        groupChatMemberRepository.save(member);

        groupChat.setUpdatedAt(Instant.now());
        groupChatRepository.save(groupChat);

        return new RemoveMemberResponse(targetUserId.toString(), true);
    }

    @Transactional
    public UpdateGroupChatResponse updateGroupChat(UUID currentUserId, Long groupChatId, UpdateGroupChatRequest request) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureAdmin(currentUserId, groupChatId);

        if (request.name() != null && !request.name().isBlank()) {
            groupChat.setName(request.name().trim());
        }
        if (request.description() != null) {
            groupChat.setDescription(request.description().trim());
        }
        if (request.maxMembers() != null) {
            validateMaxMembers(request.maxMembers());

            long currentMemberCount = groupChatMemberRepository.countByGroupChatIdAndStatus(groupChatId, "JOINED");
            if (request.maxMembers() < currentMemberCount) {
                throw new ApiException(HttpStatus.FORBIDDEN, "MAX_MEMBERS_TOO_SMALL",
                        "Cannot set max members lower than current member count", null);
            }

            groupChat.setMaxMembers(request.maxMembers());
        }

        groupChat.setUpdatedAt(Instant.now());
        GroupChat saved = groupChatRepository.save(groupChat);

        return new UpdateGroupChatResponse(saved.getId(), saved.getName(), saved.getDescription(), saved.getMaxMembers());
    }

    @Transactional
    public CloseGroupChatResponse closeGroupChat(UUID currentUserId, Long groupChatId) {
        GroupChat groupChat = groupChatRepository.findByIdAndStatus(groupChatId, "ACTIVE")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "GROUP_CHAT_NOT_FOUND",
                        "Group chat not found", null));

        ensureAdmin(currentUserId, groupChatId);

        groupChat.setStatus("CLOSED");
        groupChat.setUpdatedAt(Instant.now());
        groupChatRepository.save(groupChat);

        return new CloseGroupChatResponse(groupChatId, "CLOSED", Instant.now().toString());
    }

    private void validateUser(UUID userId) {
        UserAccount user = userAccountRepository.findById(userId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "USER_NOT_VERIFIED",
                        "User not found", null));
        if (!"VERIFIED".equals(user.getAuthenticationStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "USER_NOT_VERIFIED",
                    "Only verified users can create group chats", null);
        }
    }

    private void ensureMember(UUID userId, Long groupChatId) {
        if (groupChatMemberRepository.findActiveMember(groupChatId, userId).isEmpty()) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_MEMBER",
                    "You are not a member of this group chat", null);
        }
    }

    private void ensureAdmin(UUID userId, Long groupChatId) {
        GroupChatMember member = groupChatMemberRepository.findActiveMember(groupChatId, userId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "NOT_MEMBER",
                        "You are not a member of this group chat", null));
        if (!member.isAdmin()) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_ADMIN",
                    "Only admins can perform this action", null);
        }
    }

    private void validateMessage(String message) {
        if (message == null || message.isBlank()) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED",
                    "Message must not be blank", Map.of("field", "message"));
        }
        if (message.trim().length() > MAX_MESSAGE_LENGTH) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED",
                    "Message too long", Map.of("field", "message"));
        }
    }

    private void validateMaxMembers(int maxMembers) {
        if (maxMembers < MIN_MAX_MEMBERS || maxMembers > MAX_MAX_MEMBERS) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "INVALID_MAX_MEMBERS",
                    String.format("maxMembers must be between %d and %d", MIN_MAX_MEMBERS, MAX_MAX_MEMBERS),
                    Map.of("min", MIN_MAX_MEMBERS, "max", MAX_MAX_MEMBERS, "actual", maxMembers));
        }
    }

    private long computeUnreadCount(Long groupChatId, GroupChatMember member) {
        Long lastReadId = member.getLastReadMessageId();
        if (lastReadId == null) {
            return groupChatMessageRepository.countByGroupChatId(groupChatId);
        }
        return groupChatMessageRepository.countUnread(groupChatId, lastReadId);
    }

    private GroupChatListItem toListItem(GroupChat chat, UUID currentUserId) {
        long memberCount = groupChatMemberRepository.countByGroupChatIdAndStatus(chat.getId(), "JOINED");

        GroupChatMember currentMember = groupChatMemberRepository.findActiveMember(chat.getId(), currentUserId).orElse(null);
        String role = currentMember != null ? currentMember.getRole() : null;

        String lastMessagePreview = null;
        String lastMessageAt = null;
        List<GroupChatMessage> lastMsgs = groupChatMessageRepository.findTop1ByGroupChatIdOrderByCreatedAtDesc(chat.getId());
        if (!lastMsgs.isEmpty()) {
            GroupChatMessage lastMsg = lastMsgs.get(0);
            if (lastMsg.getContent() != null) {
                lastMessagePreview = lastMsg.getContent().length() > 30
                        ? lastMsg.getContent().substring(0, 30) + "..."
                        : lastMsg.getContent();
            }
            lastMessageAt = lastMsg.getCreatedAt().toString();
        }

        long unreadCount = currentMember != null ? computeUnreadCount(chat.getId(), currentMember) : 0;

        return new GroupChatListItem(
                chat.getId(),
                chat.getName(),
                chat.getDescription(),
                chat.getStatus(),
                memberCount,
                chat.getMaxMembers(),
                role,
                lastMessagePreview,
                lastMessageAt,
                chat.getCreatedAt().toString(),
                chat.getUpdatedAt().toString(),
                unreadCount
        );
    }

    private MemberItem toMemberItem(GroupChatMember member) {
        String displayName = userAccountRepository.findById(member.getUserId())
                .map(UserAccount::getDisplayName).orElse(null);

        return new MemberItem(
                member.getUserId().toString(),
                displayName,
                member.getRole(),
                member.getStatus(),
                member.getJoinedAt().toString()
        );
    }

    private GroupMessageItem toMessageItem(GroupChatMessage msg) {
        String senderName = userAccountRepository.findById(msg.getSenderId())
                .map(UserAccount::getDisplayName).orElse(null);

        return new GroupMessageItem(
                msg.getId(),
                msg.getSenderId().toString(),
                senderName,
                msg.getMessageType(),
                msg.getContent(),
                msg.getCreatedAt().toString()
        );
    }

    public record CreateGroupChatRequest(String name, String description, UUID relatedPostUuid,
                                         Integer maxMembers, List<UUID> initialMemberIds) {}

    public record CreateGroupChatResponse(Long groupChatId, String name, String status) {}

    public record UpdateGroupChatRequest(String name, String description, Integer maxMembers) {}

    public record UpdateGroupChatResponse(Long groupChatId, String name, String description, Integer maxMembers) {}

    public record CloseGroupChatResponse(Long groupChatId, String status, String closedAt) {}

    public record GroupChatDetailResponse(
            Long groupChatId,
            String name,
            String description,
            String status,
            Integer maxMembers,
            Integer memberCount,
            String currentUserRole,
            String lastMessagePreview,
            String lastMessageAt,
            String createdAt,
            String updatedAt,
            List<MemberItem> members,
            long unreadCount
    ) {}

    public record GroupChatListItem(
            Long groupChatId,
            String name,
            String description,
            String status,
            long memberCount,
            Integer maxMembers,
            String currentUserRole,
            String lastMessagePreview,
            String lastMessageAt,
            String createdAt,
            String updatedAt,
            long unreadCount
    ) {}

    public record GroupChatListResponse(
            List<GroupChatListItem> items,
            int page,
            int size,
            long totalElements,
            int totalPages
    ) {}

    public record MemberItem(
            String userId,
            String displayName,
            String role,
            String status,
            String joinedAt
    ) {}

    public record SendGroupMessageResponse(Long messageId, Long groupChatId) {}

    public record GroupMessageItem(
            Long messageId,
            String senderId,
            String senderName,
            String messageType,
            String content,
            String createdAt
    ) {}

    public record GroupMessageListResponse(
            List<GroupMessageItem> items,
            int page,
            int size,
            long totalElements,
            int totalPages
    ) {}

    public record AddMembersResponse(List<String> addedUserIds, List<String> alreadyMemberUserIds, List<String> invalidUserIds) {}

    public record RemoveMemberResponse(String userId, boolean success) {}
}