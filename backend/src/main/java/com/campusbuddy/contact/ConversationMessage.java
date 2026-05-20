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
@Table(name = "conversation_message")
public class ConversationMessage {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private Long conversationId;

    private UUID senderId;

    @Column(nullable = false, length = 30)
    private String messageType;

    private String content;

    @Column(nullable = false)
    private Instant createdAt;

    protected ConversationMessage() {
    }

    public ConversationMessage(Long conversationId, UUID senderId, String messageType, Instant createdAt) {
        this.conversationId = conversationId;
        this.senderId = senderId;
        this.messageType = messageType;
        this.createdAt = createdAt;
    }

    public Long getId() { return id; }
    public Long getConversationId() { return conversationId; }
    public UUID getSenderId() { return senderId; }
    public String getMessageType() { return messageType; }
    public String getContent() { return content; }
    public Instant getCreatedAt() { return createdAt; }
}
