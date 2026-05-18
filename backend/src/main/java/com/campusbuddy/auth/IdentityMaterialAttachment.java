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
@Table(name = "identity_material_attachment")
public class IdentityMaterialAttachment {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private UUID attachmentId;

    @Column(nullable = false)
    private UUID ownerUserId;

    @Column(nullable = false, length = 500, unique = true)
    private String objectKey;

    @Column(nullable = false, length = 100)
    private String contentType;

    @Column(nullable = false, length = 255)
    private String originalFilename;

    @Column(nullable = false)
    private long sizeBytes;

    @Column(nullable = false, length = 64)
    private String sha256;

    @Column(nullable = false, length = 30)
    private String businessType = "IDENTITY_MATERIAL";

    @Column(nullable = false, length = 30)
    private String status = "ACTIVE";

    @Column(nullable = false)
    private Instant createdAt;

    protected IdentityMaterialAttachment() {
    }

    public IdentityMaterialAttachment(UUID ownerUserId, String objectKey, String contentType,
                                      String originalFilename, long sizeBytes, String sha256, Instant createdAt) {
        this.ownerUserId = ownerUserId;
        this.objectKey = objectKey;
        this.contentType = contentType;
        this.originalFilename = originalFilename;
        this.sizeBytes = sizeBytes;
        this.sha256 = sha256;
        this.createdAt = createdAt;
    }

    public UUID getAttachmentId() { return attachmentId; }
    public UUID getOwnerUserId() { return ownerUserId; }
    public String getObjectKey() { return objectKey; }
    public String getContentType() { return contentType; }
    public String getOriginalFilename() { return originalFilename; }
    public long getSizeBytes() { return sizeBytes; }
    public String getSha256() { return sha256; }
    public String getBusinessType() { return businessType; }
    public String getStatus() { return status; }
    public Instant getCreatedAt() { return createdAt; }

    public void setStatus(String status) { this.status = status; }
}
