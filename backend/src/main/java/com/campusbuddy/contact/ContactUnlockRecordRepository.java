package com.campusbuddy.contact;

import org.springframework.data.jpa.repository.JpaRepository;

public interface ContactUnlockRecordRepository extends JpaRepository<ContactUnlockRecord, Long> {
    boolean existsByConversationId(Long conversationId);
}
