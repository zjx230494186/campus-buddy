package com.campusbuddy.post;

import jakarta.persistence.*;
import org.hibernate.annotations.JdbcTypeCode;
import org.hibernate.type.SqlTypes;

import java.time.Instant;
import java.util.List;
import java.util.Map;
import java.util.UUID;

@Entity
@Table(name = "partner_post")
public class PartnerPost {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private UUID id;

    @Column(nullable = false)
    private UUID publisherId;

    @Column(length = 30)
    private String sceneType;

    @Column(nullable = false, length = 30)
    private String status = "DRAFT";

    @Column(length = 40)
    private String title;

    @Column(length = 500)
    private String description;

    @Column(length = 30)
    private String timeMode;

    @Column(length = 100)
    private String timeText;

    @Column
    private Instant startAt;

    @Column
    private Instant endAt;

    @Column(length = 80)
    private String locationText;

    @Column
    private Integer participantCount;

    @Column(length = 120)
    private String targetRequirement;

    @Column(length = 80)
    private String contactPreference;

    @JdbcTypeCode(SqlTypes.JSON)
    @Column(columnDefinition = "jsonb")
    private List<String> tags;

    @JdbcTypeCode(SqlTypes.JSON)
    @Column(columnDefinition = "jsonb")
    private List<String> attachmentIds;

    @JdbcTypeCode(SqlTypes.JSON)
    @Column(columnDefinition = "jsonb")
    private Map<String, Object> scenePayload;

    @Column(length = 500)
    private String rejectReason;

    private UUID reviewedBy;

    private Instant reviewedAt;

    private Instant publishedAt;

    @Column(nullable = false)
    private Instant createdAt;

    @Column(nullable = false)
    private Instant updatedAt;

    protected PartnerPost() {
    }

    public PartnerPost(UUID publisherId, String status, Instant createdAt) {
        this.publisherId = publisherId;
        this.status = status;
        this.createdAt = createdAt;
        this.updatedAt = createdAt;
    }

    public UUID getId() { return id; }
    public UUID getPublisherId() { return publisherId; }
    public String getSceneType() { return sceneType; }
    public String getStatus() { return status; }
    public String getTitle() { return title; }
    public String getDescription() { return description; }
    public String getTimeMode() { return timeMode; }
    public String getTimeText() { return timeText; }
    public Instant getStartAt() { return startAt; }
    public Instant getEndAt() { return endAt; }
    public String getLocationText() { return locationText; }
    public Integer getParticipantCount() { return participantCount; }
    public String getTargetRequirement() { return targetRequirement; }
    public String getContactPreference() { return contactPreference; }
    public List<String> getTags() { return tags; }
    public List<String> getAttachmentIds() { return attachmentIds; }
    public Map<String, Object> getScenePayload() { return scenePayload; }
    public String getRejectReason() { return rejectReason; }
    public UUID getReviewedBy() { return reviewedBy; }
    public Instant getReviewedAt() { return reviewedAt; }
    public Instant getPublishedAt() { return publishedAt; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }

    public void setSceneType(String sceneType) { this.sceneType = sceneType; }
    public void setStatus(String status) { this.status = status; }
    public void setTitle(String title) { this.title = title; }
    public void setDescription(String description) { this.description = description; }
    public void setTimeMode(String timeMode) { this.timeMode = timeMode; }
    public void setTimeText(String timeText) { this.timeText = timeText; }
    public void setStartAt(Instant startAt) { this.startAt = startAt; }
    public void setEndAt(Instant endAt) { this.endAt = endAt; }
    public void setLocationText(String locationText) { this.locationText = locationText; }
    public void setParticipantCount(Integer participantCount) { this.participantCount = participantCount; }
    public void setTargetRequirement(String targetRequirement) { this.targetRequirement = targetRequirement; }
    public void setContactPreference(String contactPreference) { this.contactPreference = contactPreference; }
    public void setTags(List<String> tags) { this.tags = tags; }
    public void setAttachmentIds(List<String> attachmentIds) { this.attachmentIds = attachmentIds; }
    public void setScenePayload(Map<String, Object> scenePayload) { this.scenePayload = scenePayload; }
    public void setUpdatedAt(Instant updatedAt) { this.updatedAt = updatedAt; }
    public void setPublishedAt(Instant publishedAt) { this.publishedAt = publishedAt; }
}
