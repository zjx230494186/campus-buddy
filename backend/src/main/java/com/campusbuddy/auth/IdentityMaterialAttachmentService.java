package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import com.campusbuddy.storage.ObjectStorageService;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.time.Instant;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.Set;
import java.util.UUID;

@Service
class IdentityMaterialAttachmentService {

    private static final long MAX_SIZE_BYTES = 10 * 1024 * 1024;
    private static final Set<String> ALLOWED_CONTENT_TYPES = Set.of(
            "image/jpeg", "image/png", "application/pdf"
    );
    private static final DateTimeFormatter KEY_DATE_FORMAT = DateTimeFormatter.ofPattern("yyyy/MM/dd");

    private final IdentityMaterialAttachmentRepository attachmentRepository;
    private final UserAccountRepository userAccountRepository;
    private final IdentityVerificationSubmissionRepository submissionRepository;
    private final ObjectStorageService objectStorageService;

    IdentityMaterialAttachmentService(IdentityMaterialAttachmentRepository attachmentRepository,
                                      UserAccountRepository userAccountRepository,
                                      IdentityVerificationSubmissionRepository submissionRepository,
                                      ObjectStorageService objectStorageService) {
        this.attachmentRepository = attachmentRepository;
        this.userAccountRepository = userAccountRepository;
        this.submissionRepository = submissionRepository;
        this.objectStorageService = objectStorageService;
    }

    @Transactional
    UploadResponse upload(UUID userId, String contentType, String originalFilename, byte[] data) {
        userAccountRepository.findById(userId)
                .orElseThrow(() -> new ApiException(HttpStatus.UNAUTHORIZED, "UNAUTHORIZED", "User not found", "invalid userId"));

        if (!ALLOWED_CONTENT_TYPES.contains(contentType)) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "ATTACHMENT_TYPE_NOT_ALLOWED",
                    "Attachment type not allowed", "allowed types: image/jpeg, image/png, application/pdf");
        }

        if (data.length > MAX_SIZE_BYTES) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "ATTACHMENT_TOO_LARGE",
                    "Attachment too large", "maximum size is 10MB");
        }

        String safeFilename = sanitizeFilename(originalFilename);
        String sha256 = computeSha256(data);
        String objectKey = generateObjectKey();

        objectStorageService.putObject(objectKey, contentType, data);

        IdentityMaterialAttachment attachment = new IdentityMaterialAttachment(
                userId, objectKey, contentType, safeFilename, data.length, sha256, Instant.now()
        );
        attachmentRepository.save(attachment);

        return new UploadResponse(
                attachment.getAttachmentId().toString(),
                contentType,
                data.length,
                sha256,
                "ACTIVE"
        );
    }

    IdentityMaterialAttachment findForAdmin(UUID attachmentId) {
        return attachmentRepository.findById(attachmentId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                        "Attachment not found", "attachmentId does not exist"));
    }

    @Transactional
    void delete(UUID userId, UUID attachmentId) {
        IdentityMaterialAttachment attachment = attachmentRepository
                .findByAttachmentIdAndOwnerUserId(attachmentId, userId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                        "Attachment not found", "attachmentId does not exist or not owned by user"));

        if ("DELETED".equals(attachment.getStatus())) {
            throw new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                    "Attachment not found", "attachment already deleted");
        }

        if (submissionRepository.existsByMaterialAttachmentId(attachmentId)) {
            throw new ApiException(HttpStatus.CONFLICT, "ATTACHMENT_REFERENCED",
                    "Attachment is referenced by a verification submission", "cannot delete referenced attachment");
        }

        objectStorageService.deleteObject(attachment.getObjectKey());
        attachment.setStatus("DELETED");
        attachmentRepository.save(attachment);
    }

    private String generateObjectKey() {
        Instant now = Instant.now();
        String datePart = now.atZone(ZoneId.of("UTC")).format(KEY_DATE_FORMAT);
        return "auth-materials/" + datePart + "/" + UUID.randomUUID();
    }

    private String sanitizeFilename(String filename) {
        if (filename == null || filename.isBlank()) return "unnamed";
        String sanitized = filename.replaceAll("[\\\\/]", "_");
        if (sanitized.length() > 200) {
            sanitized = sanitized.substring(0, 200);
        }
        return sanitized;
    }

    private String computeSha256(byte[] data) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(data);
            StringBuilder hex = new StringBuilder();
            for (byte b : hash) {
                hex.append(String.format("%02x", b));
            }
            return hex.toString();
        } catch (Exception e) {
            throw new RuntimeException("Failed to compute SHA-256", e);
        }
    }

    record UploadResponse(String attachmentId, String contentType, int sizeBytes, String sha256, String status) {}
}
