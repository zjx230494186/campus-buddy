package com.campusbuddy.auth;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface IdentityVerificationSubmissionRepository extends JpaRepository<IdentityVerificationSubmission, UUID> {
    Optional<IdentityVerificationSubmission> findByUserId(UUID userId);
    boolean existsByUserId(UUID userId);
    List<IdentityVerificationSubmission> findByReviewStatus(String reviewStatus);
    boolean existsByMaterialAttachmentId(UUID materialAttachmentId);
}
