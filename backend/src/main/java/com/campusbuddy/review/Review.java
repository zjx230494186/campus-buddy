package com.campusbuddy.review;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "review")
public class Review {

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false)
    private Long conversationId;

    @Column(nullable = false)
    private UUID reviewerId;

    @Column(nullable = false)
    private UUID revieweeId;

    @Column(nullable = false)
    private int rating;

    private String reviewTags;

    @Column(nullable = false, length = 30)
    private String status = "ACTIVE";

    @Column(nullable = false)
    private boolean modifiedOnce = false;

    @Column(nullable = false)
    private Instant createdAt;

    @Column(nullable = false)
    private Instant updatedAt;

    protected Review() {
    }

    public Review(Long conversationId, UUID reviewerId, UUID revieweeId, int rating, String reviewTags, Instant createdAt) {
        this.conversationId = conversationId;
        this.reviewerId = reviewerId;
        this.revieweeId = revieweeId;
        this.rating = rating;
        this.reviewTags = reviewTags;
        this.createdAt = createdAt;
        this.updatedAt = createdAt;
    }

    public Long getId() { return id; }
    public Long getConversationId() { return conversationId; }
    public UUID getReviewerId() { return reviewerId; }
    public UUID getRevieweeId() { return revieweeId; }
    public int getRating() { return rating; }
    public String getReviewTags() { return reviewTags; }
    public String getStatus() { return status; }
    public boolean isModifiedOnce() { return modifiedOnce; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }

    public void update(int rating, String reviewTags) {
        this.rating = rating;
        this.reviewTags = reviewTags;
        this.modifiedOnce = true;
        this.updatedAt = Instant.now();
    }
}
