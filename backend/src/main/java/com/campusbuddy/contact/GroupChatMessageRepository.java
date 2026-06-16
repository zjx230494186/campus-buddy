package com.campusbuddy.contact;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;
import java.util.UUID;

public interface GroupChatMessageRepository extends JpaRepository<GroupChatMessage, Long> {

    Page<GroupChatMessage> findByGroupChatIdOrderByCreatedAtAsc(Long groupChatId, Pageable pageable);

    Page<GroupChatMessage> findByGroupChatIdAndIdGreaterThanOrderByIdAsc(Long groupChatId, Long afterMessageId, Pageable pageable);

    List<GroupChatMessage> findTop1ByGroupChatIdOrderByCreatedAtDesc(Long groupChatId);

    long countByGroupChatId(Long groupChatId);

    @Query("SELECT COUNT(m) FROM GroupChatMessage m WHERE m.groupChatId = :groupChatId AND m.id > :lastReadMessageId")
    long countUnread(@Param("groupChatId") Long groupChatId, @Param("lastReadMessageId") Long lastReadMessageId);

    long countByGroupChatIdAndSenderId(Long groupChatId, UUID senderId);
}