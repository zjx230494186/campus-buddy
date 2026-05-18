package com.campusbuddy.auth;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.Instant;

@Entity
@Table(name = "campus_email_verification_code")
public class CampusEmailVerificationCodeEntity {

    @Id
    @Column(length = 255)
    private String emailPurposeKey;

    @Column(nullable = false, length = 255)
    private String campusEmail;

    @Column(nullable = false, length = 64)
    private String codeHash;

    @Column(nullable = false)
    private Instant expiresAt;

    @Column(nullable = false)
    private Instant nextAllowedAt;

    @Column(nullable = false)
    private Instant createdAt;

    protected CampusEmailVerificationCodeEntity() {
    }

    public CampusEmailVerificationCodeEntity(String emailPurposeKey, String campusEmail, String codeHash, Instant expiresAt, Instant nextAllowedAt, Instant createdAt) {
        this.emailPurposeKey = emailPurposeKey;
        this.campusEmail = campusEmail;
        this.codeHash = codeHash;
        this.expiresAt = expiresAt;
        this.nextAllowedAt = nextAllowedAt;
        this.createdAt = createdAt;
    }

    public String getEmailPurposeKey() { return emailPurposeKey; }
    public String getCampusEmail() { return campusEmail; }
    public String getCodeHash() { return codeHash; }
    public Instant getExpiresAt() { return expiresAt; }
    public Instant getNextAllowedAt() { return nextAllowedAt; }
    public Instant getCreatedAt() { return createdAt; }
}
