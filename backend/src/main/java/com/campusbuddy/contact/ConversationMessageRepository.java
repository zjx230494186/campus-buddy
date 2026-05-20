package com.campusbuddy.contact;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.UUID;

public interface ConversationMessageRepository extends JpaRepository<ConversationMessage, Long> {
    long countByConversationIdAndSenderIdAndMessageType(Long conversationId, UUID senderId, String messageType);
}
