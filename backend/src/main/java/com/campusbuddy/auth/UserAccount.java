package com.campusbuddy.auth;

import jakarta.persistence.Column;
import jakarta.persistence.Entity;
import jakarta.persistence.GeneratedValue;
import jakarta.persistence.GenerationType;
import jakarta.persistence.Id;
import jakarta.persistence.Table;

import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "user_account")
public class UserAccount {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private UUID userId;

    @Column(nullable = false, unique = true, length = 255)
    private String campusEmail;

    @Column(nullable = false, length = 255)
    private String passwordHash;

    @Column(nullable = false, length = 100)
    private String displayName;

    @Column(nullable = false, length = 30)
    private String authenticationStatus = "UNVERIFIED";

    @Column(nullable = false, length = 30)
    private String campusEmailVerificationStatus = "UNVERIFIED";

    @Column(nullable = false, length = 30)
    private String accountRole = "STUDENT";

    @Column(nullable = false)
    private Instant createdAt;

    @Column(nullable = false)
    private Instant updatedAt;

    protected UserAccount() {
    }

    public UserAccount(String campusEmail, String passwordHash, String displayName, Instant createdAt) {
        this.campusEmail = campusEmail;
        this.passwordHash = passwordHash;
        this.displayName = displayName;
        this.createdAt = createdAt;
        this.updatedAt = createdAt;
    }

    public UUID getUserId() { return userId; }
    public String getCampusEmail() { return campusEmail; }
    public String getPasswordHash() { return passwordHash; }
    public String getDisplayName() { return displayName; }
    public String getAuthenticationStatus() { return authenticationStatus; }
    public String getCampusEmailVerificationStatus() { return campusEmailVerificationStatus; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }
    public String getAccountRole() { return accountRole; }

    public void setCampusEmailVerificationStatus(String status) {
        this.campusEmailVerificationStatus = status;
        this.updatedAt = Instant.now();
    }

    public void setAuthenticationStatus(String authenticationStatus) {
        this.authenticationStatus = authenticationStatus;
        this.updatedAt = Instant.now();
    }

    public void setAccountRole(String accountRole) {
        this.accountRole = accountRole;
        this.updatedAt = Instant.now();
    }
}
