package com.campusbuddy.contact;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.UUID;

public interface ConversationMessageRepository extends JpaRepository<ConversationMessage, Long> {
    long countByConversationIdAndSenderIdAndMessageType(Long conversationId, UUID senderId, String messageType);

    Page<ConversationMessage> findByConversationIdOrderByCreatedAtAsc(Long conversationId, Pageable pageable);

    Page<ConversationMessage> findByConversationIdAndIdGreaterThanOrderByIdAsc(Long conversationId, Long afterMessageId, Pageable pageable);

    List<ConversationMessage> findByConversationIdAndIdGreaterThanOrderByIdAsc(Long conversationId, Long afterMessageId);

    List<ConversationMessage> findTop1ByConversationIdOrderByCreatedAtDesc(Long conversationId);
}
