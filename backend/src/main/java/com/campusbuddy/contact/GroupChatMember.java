package com.campusbuddy.contact;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;
import jakarta.persistence.UniqueConstraint;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "group_chat_member", uniqueConstraints = {
        @UniqueConstraint(columnNames = {"group_chat_id", "user_id"})
})
public class GroupChatMember {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(name = "group_chat_id", nullable = false)
    private Long groupChatId;

    @Column(name = "user_id", nullable = false)
    private UUID userId;

    @Column(nullable = false, length = 30)
    private String role = "MEMBER";

    @Column(nullable = false, length = 30)
    private String status = "JOINED";

    @Column(name = "joined_at", nullable = false)
    private Instant joinedAt;

    @Column(name = "last_read_message_id")
    private Long lastReadMessageId;

    @Column(name = "left_at")
    private Instant leftAt;

    protected GroupChatMember() {
    }

    public GroupChatMember(Long groupChatId, UUID userId, String role, Instant joinedAt) {
        this.groupChatId = groupChatId;
        this.userId = userId;
        this.role = role;
        this.status = "JOINED";
        this.joinedAt = joinedAt;
    }

    public Long getId() { return id; }
    public Long getGroupChatId() { return groupChatId; }
    public UUID getUserId() { return userId; }
    public String getRole() { return role; }
    public String getStatus() { return status; }
    public Instant getJoinedAt() { return joinedAt; }
    public Long getLastReadMessageId() { return lastReadMessageId; }
    public Instant getLeftAt() { return leftAt; }

    public void setRole(String role) { this.role = role; }
    public void setStatus(String status) { this.status = status; }
    public void setLastReadMessageId(Long lastReadMessageId) { this.lastReadMessageId = lastReadMessageId; }
    public void setLeftAt(Instant leftAt) { this.leftAt = leftAt; }

    public boolean isAdmin() {
        return "ADMIN".equals(role);
    }

    public boolean isJoined() {
        return "JOINED".equals(status);
    }
}