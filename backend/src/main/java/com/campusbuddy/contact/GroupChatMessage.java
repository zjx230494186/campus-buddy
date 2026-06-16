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
@Table(name = "group_chat_message")
public class GroupChatMessage {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name = "group_chat_id", nullable = false)
    private Long groupChatId;

    @Column(name = "sender_id", nullable = false)
    private UUID senderId;

    @Column(nullable = false, length = 30)
    private String messageType;

    @Column(length = 1000)
    private String content;

    @Column(name = "created_at", nullable = false)
    private Instant createdAt;

    protected GroupChatMessage() {
    }

    public GroupChatMessage(Long groupChatId, UUID senderId, String messageType, String content, Instant createdAt) {
        this.groupChatId = groupChatId;
        this.senderId = senderId;
        this.messageType = messageType;
        this.content = content;
        this.createdAt = createdAt;
    }

    public Long getId() { return id; }
    public Long getGroupChatId() { return groupChatId; }
    public UUID getSenderId() { return senderId; }
    public String getMessageType() { return messageType; }
    public String getContent() { return content; }
    public Instant getCreatedAt() { return createdAt; }
}