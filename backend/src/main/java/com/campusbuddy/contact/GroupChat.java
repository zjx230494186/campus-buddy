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
@Table(name = "group_chat")
public class GroupChat {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false, length = 100)
    private String name;

    @Column(length = 500)
    private String description;

    @Column(name = "creator_id", nullable = false)
    private UUID creatorId;

    @Column(name = "related_post_uuid")
    private UUID relatedPostUuid;

    @Column(nullable = false, length = 30)
    private String status = "ACTIVE";

    @Column(name = "max_members", nullable = false)
    private Integer maxMembers = 20;

    @Column(name = "created_at", nullable = false)
    private Instant createdAt;

    @Column(name = "updated_at", nullable = false)
    private Instant updatedAt;

    @Column(name = "last_message_at")
    private Instant lastMessageAt;

    protected GroupChat() {
    }

    public GroupChat(String name, String description, UUID creatorId, UUID relatedPostUuid, Instant createdAt) {
        this.name = name;
        this.description = description;
        this.creatorId = creatorId;
        this.relatedPostUuid = relatedPostUuid;
        this.createdAt = createdAt;
        this.updatedAt = createdAt;
        this.lastMessageAt = createdAt;
    }

    public Long getId() { return id; }
    public String getName() { return name; }
    public String getDescription() { return description; }
    public UUID getCreatorId() { return creatorId; }
    public UUID getRelatedPostUuid() { return relatedPostUuid; }
    public String getStatus() { return status; }
    public Integer getMaxMembers() { return maxMembers; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }
    public Instant getLastMessageAt() { return lastMessageAt; }

    public void setName(String name) { this.name = name; }
    public void setDescription(String description) { this.description = description; }
    public void setStatus(String status) { this.status = status; }
    public void setMaxMembers(Integer maxMembers) { this.maxMembers = maxMembers; }
    public void setUpdatedAt(Instant updatedAt) { this.updatedAt = updatedAt; }
    public void setLastMessageAt(Instant lastMessageAt) { this.lastMessageAt = lastMessageAt; }
}