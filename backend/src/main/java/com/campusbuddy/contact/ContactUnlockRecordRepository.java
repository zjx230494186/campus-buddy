package com.campusbuddy.contact;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;

public interface ContactUnlockRecordRepository extends JpaRepository<ContactUnlockRecord, Long> {
    boolean existsByConversationId(Long conversationId);
    Optional<ContactUnlockRecord> findByConversationId(Long conversationId);
}
