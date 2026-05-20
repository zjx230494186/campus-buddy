package com.campusbuddy.contact;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "conversation")
public class Conversation {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private UUID participant1Id;

    @Column(nullable = false)
    private UUID participant2Id;

    @Column(nullable = false, length = 30)
    private String status = "ACTIVE";

    private Long relatedPostId;

    @Column(nullable = false)
    private Instant createdAt;

    @Column(nullable = false)
    private Instant updatedAt;

    protected Conversation() {
    }

    public Conversation(UUID participant1Id, UUID participant2Id, String status, Instant createdAt) {
        if (participant1Id.equals(participant2Id)) {
            throw new IllegalArgumentException("participant1Id and participant2Id must be different");
        }
        this.participant1Id = participant1Id;
        this.participant2Id = participant2Id;
        this.status = status;
        this.createdAt = createdAt;
        this.updatedAt = createdAt;
    }

    public Long getId() { return id; }
    public UUID getParticipant1Id() { return participant1Id; }
    public UUID getParticipant2Id() { return participant2Id; }
    public String getStatus() { return status; }
    public Long getRelatedPostId() { return relatedPostId; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }

    public boolean isParticipant(UUID userId) {
        return participant1Id.equals(userId) || participant2Id.equals(userId);
    }

    public UUID getOtherParticipant(UUID userId) {
        if (participant1Id.equals(userId)) return participant2Id;
        if (participant2Id.equals(userId)) return participant1Id;
        return null;
    }
}
