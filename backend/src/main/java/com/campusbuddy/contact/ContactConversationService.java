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
                    Conversation conv = new Conversation(currentUserId, publisherId, "ACTIVE", Instant.now());
                    return conversationRepository.save(conv);
                });

        conversation.setRelatedPostUuid(postId);
        conversation.setUpdatedAt(Instant.now());
        conversationRepository.save(conversation);

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
            throw new ApiException(HttpStatus.FORBIDDEN, "CONVERSATION_NOT_ACTIVE",
                    "Conversation is not active", null);
        }

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

        return new ConversationListItem(
                conv.getId(),
                conv.getStatus(),
                otherId != null ? otherId.toString() : null,
                otherDisplayName,
                relatedPostUuid,
                relatedPostTitle,
                lastMessagePreview,
                lastMessageAt,
                conv.getUpdatedAt().toString()
        );
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

    public record ConversationListItem(
            Long conversationId, String status,
            String otherParticipantId, String otherParticipantDisplayName,
            String relatedPostUuid, String relatedPostTitle,
            String lastMessagePreview, String lastMessageAt,
            String updatedAt
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
