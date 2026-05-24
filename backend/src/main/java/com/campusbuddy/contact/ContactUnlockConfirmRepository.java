package com.campusbuddy.contact;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.UUID;

public interface ContactUnlockConfirmRepository extends JpaRepository<ContactUnlockConfirm, Long> {
    boolean existsByConversationIdAndUserId(Long conversationId, UUID userId);
    List<ContactUnlockConfirm> findByConversationId(Long conversationId);
    long countByConversationId(Long conversationId);
}
