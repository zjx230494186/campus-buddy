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

@Service
public class ContactConversationService {

    private static final int MAX_MESSAGE_LENGTH = 30;
    private static final int MAX_PAGE_SIZE = 50;

    private final ConversationRepository conversationRepository;
    private final ConversationMessageRepository conversationMessageRepository;
    private final PartnerPostRepository partnerPostRepository;
    private final UserAccountRepository userAccountRepository;

    ContactConversationService(ConversationRepository conversationRepository,
                               ConversationMessageRepository conversationMessageRepository,
                               PartnerPostRepository partnerPostRepository,
                               UserAccountRepository userAccountRepository) {
        this.conversationRepository = conversationRepository;
        this.conversationMessageRepository = conversationMessageRepository;
        this.partnerPostRepository = partnerPostRepository;
        this.userAccountRepository = userAccountRepository;
    }

    @Transactional
    public ContactRequestResponse requestContact(UUID currentUserId, UUID postId, String message) {
        validateMessage(message);

        PartnerPost post = partnerPostRepository.findByIdAndStatus(postId, "PUBLISHED")
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Post not found", "postId does not exist or is not published"));

        UUID publisherId = post.getPublisherId();

        if (currentUserId.equals(publisherId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "CANNOT_CONTACT_OWN_POST",
                    "Cannot contact your own post", null);
        }

        UserAccount requester = userAccountRepository.findById(currentUserId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "USER_NOT_VERIFIED",
                        "Requester not verified", null));
        if (!"VERIFIED".equals(requester.getAuthenticationStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "USER_NOT_VERIFIED",
                    "Only verified users can request contact", null);
        }

        UserAccount publisher = userAccountRepository.findById(publisherId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "PUBLISHER_NOT_VERIFIED",
                        "Publisher not verified", null));
        if (!"VERIFIED".equals(publisher.getAuthenticationStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "PUBLISHER_NOT_VERIFIED",
                    "Publisher is no longer verified", null);
        }

        Conversation conversation = conversationRepository.findActiveByParticipants(currentUserId, publisherId)
                .orElseGet(() -> {
                    Optional<Conversation> closedConv = conversationRepository.findByParticipants(currentUserId, publisherId)
                            .filter(c -> "CLOSED".equals(c.getStatus()));
                    if (closedConv.isPresent()) {
                        Conversation conv = closedConv.get();
                        conv.setStatus("ACTIVE");
                        conv.setUpdatedAt(Instant.now());
                        return conversationRepository.save(conv);
                    }
                    Conversation conv = new Conversation(currentUserId, publisherId, "ACTIVE", Instant.now());
                    return conversationRepository.save(conv);
                });

        conversation.setRelatedPostUuid(postId);
        conversation.setUpdatedAt(Instant.now());
        conversationRepository.save(conversation);

        ensurePeerHasRepliedBeforeAdditionalMessage(conversation, currentUserId);

        ConversationMessage msg = new ConversationMessage(
                conversation.getId(), currentUserId, "USER_TEXT", message.trim(), Instant.now());
        conversationMessageRepository.save(msg);

        return new ContactRequestResponse(conversation.getId(), conversation.getStatus());
    }

    @Transactional
    public SendMessageResponse sendMessage(UUID currentUserId, Long conversationId, String message) {
        validateMessage(message);

        Conversation conversation = conversationRepository.findById(conversationId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "CONVERSATION_NOT_FOUND",
                        "Conversation not found", null));

        if (!conversation.isParticipant(currentUserId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_PARTICIPANT",
                    "You are not a participant of this conversation", null);
        }

        if (!"ACTIVE".equals(conversation.getStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "CONVERSATION_CLOSED",
                    "Conversation is closed", null);
        }

        ensurePeerHasRepliedBeforeAdditionalMessage(conversation, currentUserId);

        ConversationMessage msg = new ConversationMessage(
                conversationId, currentUserId, "USER_TEXT", message.trim(), Instant.now());
        ConversationMessage saved = conversationMessageRepository.save(msg);

        conversation.setUpdatedAt(Instant.now());
        conversationRepository.save(conversation);

        return new SendMessageResponse(saved.getId());
    }

    @Transactional(readOnly = true)
    public ConversationListResponse listConversations(UUID currentUserId, int page, int size) {
        int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
        PageRequest pageable = PageRequest.of(page, safeSize);

        Page<Conversation> convPage = conversationRepository.findByParticipantOrderByUpdatedAtDesc(
                currentUserId, pageable);

        List<ConversationListItem> items = convPage.getContent().stream()
                .map(conv -> toListItem(conv, currentUserId))
                .toList();

        return new ConversationListResponse(items, convPage.getNumber(), convPage.getSize(),
                convPage.getTotalElements(), convPage.getTotalPages());
    }

    @Transactional
    public CloseConversationResponse closeConversation(UUID currentUserId, Long conversationId) {
        Conversation conversation = conversationRepository.findById(conversationId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "CONVERSATION_NOT_FOUND",
                        "Conversation not found", null));

        if (!conversation.isParticipant(currentUserId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_PARTICIPANT",
                    "You are not a participant of this conversation", null);
        }

        UUID relatedPostUuid = conversation.getRelatedPostUuid();
        if (relatedPostUuid == null) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NO_RELATED_POST",
                    "This conversation has no related post", null);
        }

        PartnerPost post = partnerPostRepository.findById(relatedPostUuid)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "POST_NOT_FOUND",
                        "Related post not found", null));

        if (!post.getPublisherId().equals(currentUserId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_POST_PUBLISHER",
                    "Only the post publisher can close this conversation", null);
        }

        if (!"CLOSED".equals(conversation.getStatus())) {
            conversation.setStatus("CLOSED");
            conversation.setCloserId(currentUserId);
            conversation.setClosedAt(Instant.now());
            conversation.setUpdatedAt(Instant.now());
            conversationRepository.save(conversation);
        }

        String otherParticipantId = conversation.getOtherParticipant(currentUserId) != null
                ? conversation.getOtherParticipant(currentUserId).toString()
                : null;

        String closedAtStr = conversation.getClosedAt() != null
                ? conversation.getClosedAt().toString()
                : null;

        return new CloseConversationResponse(conversation.getId(), conversation.getStatus(),
                closedAtStr, otherParticipantId);
    }

    @Transactional(readOnly = true)
    public CanCloseResponse canCloseConversation(UUID currentUserId, Long conversationId) {
        Conversation conversation = conversationRepository.findById(conversationId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "CONVERSATION_NOT_FOUND",
                        "Conversation not found", null));

        if (!conversation.isParticipant(currentUserId)) {
            return new CanCloseResponse(false, "NOT_PARTICIPANT");
        }

        if ("CLOSED".equals(conversation.getStatus())) {
            return new CanCloseResponse(false, "ALREADY_CLOSED");
        }

        UUID relatedPostUuid = conversation.getRelatedPostUuid();
        if (relatedPostUuid == null) {
            return new CanCloseResponse(false, "NO_RELATED_POST");
        }

        return partnerPostRepository.findById(relatedPostUuid)
                .map(post -> post.getPublisherId().equals(currentUserId)
                        ? new CanCloseResponse(true, "OK")
                        : new CanCloseResponse(false, "NOT_POST_PUBLISHER"))
                .orElse(new CanCloseResponse(false, "POST_NOT_FOUND"));
    }

    @Transactional
    public void markConversationRead(UUID currentUserId, Long conversationId) {
        Conversation conversation = conversationRepository.findById(conversationId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "CONVERSATION_NOT_FOUND",
                        "Conversation not found", null));

        if (!conversation.isParticipant(currentUserId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_PARTICIPANT",
                    "You are not a participant of this conversation", null);
        }

        var lastMsg = conversationMessageRepository.findTop1ByConversationIdOrderByIdDesc(conversationId);
        if (lastMsg.isPresent()) {
            conversation.setLastReadMessageId(currentUserId, lastMsg.get().getId());
            conversationRepository.save(conversation);
        }
    }

    @Transactional(readOnly = true)
    public MessageListResponse getMessages(UUID currentUserId, Long conversationId,
                                            Long afterMessageId, int page, int size) {
        Conversation conversation = conversationRepository.findById(conversationId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "CONVERSATION_NOT_FOUND",
                        "Conversation not found", null));

        if (!conversation.isParticipant(currentUserId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_PARTICIPANT",
                    "You are not a participant of this conversation", null);
        }

        if (afterMessageId != null && afterMessageId > 0) {
            int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
            PageRequest pageable = PageRequest.of(0, safeSize);
            Page<ConversationMessage> msgPage = conversationMessageRepository
                    .findByConversationIdAndIdGreaterThanOrderByIdAsc(conversationId, afterMessageId, pageable);
            List<MessageItem> msgItems = msgPage.getContent().stream().map(this::toMessageItem).toList();
            return new MessageListResponse(msgItems, msgPage.getNumber(), msgPage.getSize(),
                    msgPage.getTotalElements(), msgPage.getTotalPages());
        }

        int safeSize = Math.min(Math.max(size, 1), MAX_PAGE_SIZE);
        PageRequest pageable = PageRequest.of(page, safeSize);
        Page<ConversationMessage> msgPage = conversationMessageRepository
                .findByConversationIdOrderByCreatedAtAsc(conversationId, pageable);

        List<MessageItem> msgItems = msgPage.getContent().stream().map(this::toMessageItem).toList();
        return new MessageListResponse(msgItems, msgPage.getNumber(), msgPage.getSize(),
                msgPage.getTotalElements(), msgPage.getTotalPages());
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

    private void ensurePeerHasRepliedBeforeAdditionalMessage(Conversation conversation, UUID currentUserId) {
        long currentUserMessages = conversationMessageRepository.countByConversationIdAndSenderIdAndMessageType(
                conversation.getId(), currentUserId, "USER_TEXT");
        long peerMessages = conversationMessageRepository.countUserTextFromOther(conversation.getId(), currentUserId);

        if (currentUserMessages > 0 && peerMessages == 0) {
            throw new ApiException(HttpStatus.FORBIDDEN, "CONTACT_REPLY_REQUIRED",
                    "Please wait for the other participant to reply before sending another message", null);
        }
    }

    private ConversationListItem toListItem(Conversation conv, UUID currentUserId) {
        UUID otherId = conv.getOtherParticipant(currentUserId);
        String otherDisplayName = null;
        if (otherId != null) {
            otherDisplayName = userAccountRepository.findById(otherId)
                    .map(UserAccount::getDisplayName).orElse(null);
        }

        String relatedPostTitle = null;
        String relatedPostUuid = null;
        if (conv.getRelatedPostUuid() != null) {
            relatedPostUuid = conv.getRelatedPostUuid().toString();
            relatedPostTitle = partnerPostRepository.findById(conv.getRelatedPostUuid())
                    .map(PartnerPost::getTitle).orElse(null);
        }

        String lastMessagePreview = null;
        String lastMessageAt = null;
        List<ConversationMessage> lastMsgs = conversationMessageRepository
                .findTop1ByConversationIdOrderByCreatedAtDesc(conv.getId());
        if (!lastMsgs.isEmpty()) {
            ConversationMessage lastMsg = lastMsgs.get(0);
            if (lastMsg.getContent() != null) {
                lastMessagePreview = lastMsg.getContent().length() > 30
                        ? lastMsg.getContent().substring(0, 30) + "..."
                        : lastMsg.getContent();
            }
            lastMessageAt = lastMsg.getCreatedAt().toString();
        }

        long unreadCount = computeUnreadCount(conv, currentUserId);

        return new ConversationListItem(
                conv.getId(),
                conv.getStatus(),
                otherId != null ? otherId.toString() : null,
                otherDisplayName,
                relatedPostUuid,
                relatedPostTitle,
                lastMessagePreview,
                lastMessageAt,
                conv.getUpdatedAt().toString(),
                unreadCount
        );
    }

    private long computeUnreadCount(Conversation conv, UUID currentUserId) {
        Long lastReadId = conv.getLastReadMessageId(currentUserId);
        if (lastReadId == null) {
            return conversationMessageRepository.countAllFromOther(conv.getId(), currentUserId);
        }
        return conversationMessageRepository.countUnread(conv.getId(), currentUserId, lastReadId);
    }

    private MessageItem toMessageItem(ConversationMessage msg) {
        return new MessageItem(
                msg.getId(),
                msg.getSenderId() != null ? msg.getSenderId().toString() : null,
                msg.getMessageType(),
                msg.getContent(),
                msg.getCreatedAt().toString()
        );
    }

    public record ContactRequestResponse(Long conversationId, String status) {}

    public record SendMessageResponse(Long messageId) {}

    public record CloseConversationResponse(Long conversationId, String status, String closedAt, String otherParticipantId) {}

    public record CanCloseResponse(boolean canClose, String reason) {}

    public record ConversationListItem(
            Long conversationId, String status,
            String otherParticipantId, String otherParticipantDisplayName,
            String relatedPostUuid, String relatedPostTitle,
            String lastMessagePreview, String lastMessageAt,
            String updatedAt, long unreadCount
    ) {}

    public record ConversationListResponse(
            List<ConversationListItem> items, int page, int size,
            long totalElements, int totalPages
    ) {}

    public record MessageItem(
            Long messageId, String senderId,
            String messageType, String content, String createdAt
    ) {}

    public record MessageListResponse(
            List<MessageItem> items, int page, int size,
            long totalElements, int totalPages
    ) {}
}
