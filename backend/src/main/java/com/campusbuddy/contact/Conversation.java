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

    @Column(name = "related_post_uuid")
    private UUID relatedPostUuid;

    @Column(nullable = false)
    private Instant createdAt;

    @Column(nullable = false)
    private Instant updatedAt;

    @Column(name = "participant1_last_read_message_id")
    private Long participant1LastReadMessageId;

    @Column(name = "participant2_last_read_message_id")
    private Long participant2LastReadMessageId;

    @Column(name = "closer_id")
    private UUID closerId;

    @Column(name = "closed_at")
    private Instant closedAt;

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
    public UUID getRelatedPostUuid() { return relatedPostUuid; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }

    public void setRelatedPostUuid(UUID relatedPostUuid) { this.relatedPostUuid = relatedPostUuid; }
    public void setUpdatedAt(Instant updatedAt) { this.updatedAt = updatedAt; }
    public void setStatus(String status) { this.status = status; }

    public Long getParticipant1LastReadMessageId() { return participant1LastReadMessageId; }
    public Long getParticipant2LastReadMessageId() { return participant2LastReadMessageId; }
    public void setParticipant1LastReadMessageId(Long id) { this.participant1LastReadMessageId = id; }
    public void setParticipant2LastReadMessageId(Long id) { this.participant2LastReadMessageId = id; }

    public UUID getCloserId() { return closerId; }
    public Instant getClosedAt() { return closedAt; }
    public void setCloserId(UUID closerId) { this.closerId = closerId; }
    public void setClosedAt(Instant closedAt) { this.closedAt = closedAt; }

    public boolean isClosedByPublisher() { return closerId != null; }

    public Long getLastReadMessageId(UUID userId) {
        if (participant1Id.equals(userId)) return participant1LastReadMessageId;
        if (participant2Id.equals(userId)) return participant2LastReadMessageId;
        return null;
    }

    public void setLastReadMessageId(UUID userId, Long messageId) {
        if (participant1Id.equals(userId)) this.participant1LastReadMessageId = messageId;
        else if (participant2Id.equals(userId)) this.participant2LastReadMessageId = messageId;
    }

    public boolean isParticipant(UUID userId) {
        return participant1Id.equals(userId) || participant2Id.equals(userId);
    }

    public UUID getOtherParticipant(UUID userId) {
        if (participant1Id.equals(userId)) return participant2Id;
        if (participant2Id.equals(userId)) return participant1Id;
        return null;
    }
}
