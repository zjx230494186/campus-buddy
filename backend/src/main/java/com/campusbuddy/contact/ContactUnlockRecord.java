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
@Table(name = "contact_unlock_record")
public class ContactUnlockRecord {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false, unique = true)
    private Long conversationId;

    @Column(nullable = false)
    private Instant unlockedAt;

    private UUID unlockedByUserId;

    @Column(nullable = false)
    private Instant createdAt;

    protected ContactUnlockRecord() {
    }

    public ContactUnlockRecord(Long conversationId, Instant unlockedAt) {
        this.conversationId = conversationId;
        this.unlockedAt = unlockedAt;
        this.createdAt = Instant.now();
    }

    public Long getId() { return id; }
    public Long getConversationId() { return conversationId; }
    public Instant getUnlockedAt() { return unlockedAt; }
    public UUID getUnlockedByUserId() { return unlockedByUserId; }
    public Instant getCreatedAt() { return createdAt; }
}
