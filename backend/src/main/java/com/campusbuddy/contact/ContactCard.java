package com.campusbuddy.contact;

import jakarta.persistence.*;
import java.time.Instant;
import java.util.UUID;

@Entity
@Table(name = "contact_card")
public class ContactCard {

    @Id @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(nullable = false, unique = true)
    private UUID userId;

    @Column(name = "wechat_id", length = 64)
    private String wechatId;

    @Column(name = "phone_number", length = 20)
    private String phoneNumber;

    @Column(name = "qq_number", length = 32)
    private String qqNumber;

    @Column(nullable = false)
    private Instant createdAt;

    @Column(nullable = false)
    private Instant updatedAt;

    protected ContactCard() {}

    public ContactCard(UUID userId, String wechatId, String phoneNumber, String qqNumber) {
        this.userId = userId;
        this.wechatId = wechatId;
        this.phoneNumber = phoneNumber;
        this.qqNumber = qqNumber;
        this.createdAt = Instant.now();
        this.updatedAt = Instant.now();
    }

    public Long getId() { return id; }
    public UUID getUserId() { return userId; }
    public String getWechatId() { return wechatId; }
    public String getPhoneNumber() { return phoneNumber; }
    public String getQqNumber() { return qqNumber; }
    public Instant getCreatedAt() { return createdAt; }
    public Instant getUpdatedAt() { return updatedAt; }

    public void setWechatId(String wechatId) { this.wechatId = wechatId; }
    public void setPhoneNumber(String phoneNumber) { this.phoneNumber = phoneNumber; }
    public void setQqNumber(String qqNumber) { this.qqNumber = qqNumber; }
    public void setUpdatedAt(Instant updatedAt) { this.updatedAt = updatedAt; }

    public boolean hasAtLeastOneContact() {
        return (wechatId != null && !wechatId.isBlank())
                || (phoneNumber != null && !phoneNumber.isBlank())
                || (qqNumber != null && !qqNumber.isBlank());
    }
}
