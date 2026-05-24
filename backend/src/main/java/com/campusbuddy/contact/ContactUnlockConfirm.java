package com.campusbuddy.contact;

import jakarta.persistence.*;
import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "contact_unlock_confirm")
public class ContactUnlockConfirm {

    @Id @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private Long conversationId;

    @Column(nullable = false)
    private UUID userId;

    @Column(nullable = false)
    private Instant confirmedAt;

    protected ContactUnlockConfirm() {}

    public ContactUnlockConfirm(Long conversationId, UUID userId) {
        this.conversationId = conversationId;
        this.userId = userId;
        this.confirmedAt = Instant.now();
    }

    public Long getId() { return id; }
    public Long getConversationId() { return conversationId; }
    public UUID getUserId() { return userId; }
    public Instant getConfirmedAt() { return confirmedAt; }
}
