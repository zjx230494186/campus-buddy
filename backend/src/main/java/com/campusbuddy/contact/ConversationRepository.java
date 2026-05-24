package com.campusbuddy.contact;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface ConversationRepository extends JpaRepository<Conversation, Long> {
    List<Conversation> findByParticipant1IdOrParticipant2Id(UUID participant1Id, UUID participant2Id);

    Page<Conversation> findByParticipant1IdOrParticipant2Id(UUID participant1Id, UUID participant2Id, Pageable pageable);

    @Query("SELECT c FROM Conversation c WHERE c.participant1Id = :userId OR c.participant2Id = :userId ORDER BY c.updatedAt DESC")
    Page<Conversation> findByParticipantOrderByUpdatedAtDesc(@Param("userId") UUID userId, Pageable pageable);

    @Query("SELECT c FROM Conversation c WHERE c.status = 'ACTIVE' AND " +
            "((c.participant1Id = :userA AND c.participant2Id = :userB) OR " +
            "(c.participant1Id = :userB AND c.participant2Id = :userA))")
    Optional<Conversation> findActiveByParticipants(@Param("userA") UUID userA, @Param("userB") UUID userB);

    @Query("SELECT c FROM Conversation c WHERE " +
            "((c.participant1Id = :userA AND c.participant2Id = :userB) OR " +
            "(c.participant1Id = :userB AND c.participant2Id = :userA))")
    Optional<Conversation> findByParticipants(@Param("userA") UUID userA, @Param("userB") UUID userB);
}
