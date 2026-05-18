package com.campusbuddy.auth;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;
import java.util.UUID;

public interface IdentityMaterialAttachmentRepository extends JpaRepository<IdentityMaterialAttachment, UUID> {
    Optional<IdentityMaterialAttachment> findByAttachmentIdAndOwnerUserId(UUID attachmentId, UUID ownerUserId);
}
