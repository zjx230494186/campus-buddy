package com.campusbuddy.contact;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface ConversationMessageRepository extends JpaRepository<ConversationMessage, Long> {
    long countByConversationIdAndSenderIdAndMessageType(Long conversationId, UUID senderId, String messageType);

    Page<ConversationMessage> findByConversationIdOrderByCreatedAtAsc(Long conversationId, Pageable pageable);

    Page<ConversationMessage> findByConversationIdAndIdGreaterThanOrderByIdAsc(Long conversationId, Long afterMessageId, Pageable pageable);

    List<ConversationMessage> findByConversationIdAndIdGreaterThanOrderByIdAsc(Long conversationId, Long afterMessageId);

    List<ConversationMessage> findTop1ByConversationIdOrderByCreatedAtDesc(Long conversationId);

    @Query("SELECT COUNT(m) FROM ConversationMessage m WHERE m.conversationId = :conversationId " +
            "AND m.senderId <> :userId AND m.id > :lastReadMessageId")
    long countUnread(@Param("conversationId") Long conversationId,
                     @Param("userId") UUID userId,
                     @Param("lastReadMessageId") Long lastReadMessageId);

    @Query("SELECT COUNT(m) FROM ConversationMessage m WHERE m.conversationId = :conversationId " +
            "AND m.senderId <> :userId")
    long countAllFromOther(@Param("conversationId") Long conversationId,
                           @Param("userId") UUID userId);

    Optional<ConversationMessage> findTop1ByConversationIdOrderByIdDesc(Long conversationId);
}
