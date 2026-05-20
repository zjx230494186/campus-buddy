package com.campusbuddy.contact;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.UUID;

public interface ConversationMessageRepository extends JpaRepository<ConversationMessage, Long> {
    long countByConversationIdAndSenderIdAndMessageTypeNot(Long conversationId, UUID senderId, String messageType);
}
