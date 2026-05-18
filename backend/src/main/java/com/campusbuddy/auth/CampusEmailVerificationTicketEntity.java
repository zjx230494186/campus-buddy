package com.campusbuddy.auth;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.Instant;

@Entity
@Table(name = "campus_email_verification_ticket")
public class CampusEmailVerificationTicketEntity {

    @Id
    @Column(length = 255)
    private String emailPurposeKey;

    @Column(nullable = false, length = 64)
    private String ticketHash;

    @Column(nullable = false)
    private Instant expiresAt;

    @Column(nullable = false)
    private Instant createdAt;

    protected CampusEmailVerificationTicketEntity() {
    }

    public CampusEmailVerificationTicketEntity(String emailPurposeKey, String ticketHash, Instant expiresAt, Instant createdAt) {
        this.emailPurposeKey = emailPurposeKey;
        this.ticketHash = ticketHash;
        this.expiresAt = expiresAt;
        this.createdAt = createdAt;
    }

    public String getEmailPurposeKey() { return emailPurposeKey; }
    public String getTicketHash() { return ticketHash; }
    public Instant getExpiresAt() { return expiresAt; }
    public Instant getCreatedAt() { return createdAt; }
}
