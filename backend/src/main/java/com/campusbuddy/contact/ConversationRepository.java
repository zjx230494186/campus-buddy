package com.campusbuddy.contact;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.UUID;

public interface ConversationRepository extends JpaRepository<Conversation, Long> {
    List<Conversation> findByParticipant1IdOrParticipant2Id(UUID participant1Id, UUID participant2Id);
}
