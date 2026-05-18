package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.List;
import java.util.UUID;

@Service
class IdentityVerificationAdminService {

    private final IdentityVerificationSubmissionRepository submissionRepository;
    private final UserAccountRepository userAccountRepository;
    private final IdentityMaterialAttachmentRepository attachmentRepository;
    private final com.campusbuddy.storage.ObjectStorageService objectStorageService;

    IdentityVerificationAdminService(IdentityVerificationSubmissionRepository submissionRepository,
                                     UserAccountRepository userAccountRepository,
                                     IdentityMaterialAttachmentRepository attachmentRepository,
                                     com.campusbuddy.storage.ObjectStorageService objectStorageService) {
        this.submissionRepository = submissionRepository;
        this.userAccountRepository = userAccountRepository;
        this.attachmentRepository = attachmentRepository;
        this.objectStorageService = objectStorageService;
    }

    List<PendingSubmissionItem> listPending() {
        return submissionRepository.findByReviewStatus("PENDING_REVIEW").stream()
                .map(sub -> {
                    String materialAttachmentId = sub.getMaterialAttachmentId() != null ? sub.getMaterialAttachmentId().toString() : null;
                    String materialContentType = null;
                    Long materialSizeBytes = null;
                    if (sub.getMaterialAttachmentId() != null) {
                        materialContentType = attachmentRepository.findById(sub.getMaterialAttachmentId())
                                .map(IdentityMaterialAttachment::getContentType).orElse(null);
                        materialSizeBytes = attachmentRepository.findById(sub.getMaterialAttachmentId())
                                .map(IdentityMaterialAttachment::getSizeBytes).orElse(null);
                    }
                    return new PendingSubmissionItem(
                            sub.getSubmissionId().toString(),
                            sub.getUserId().toString(),
                            sub.getRealName(),
                            sub.getStudentNumber(),
                            sub.getCollege(),
                            sub.getMajor(),
                            sub.getGrade(),
                            sub.getReviewStatus(),
                            sub.getSubmittedAt().toString(),
                            materialAttachmentId,
                            materialContentType,
                            materialSizeBytes
                    );
                })
                .toList();
    }

    @Transactional
    ReviewResponse review(UUID submissionId, ReviewRequest request) {
        if (request == null || request.decision() == null
                || (!"APPROVED".equals(request.decision()) && !"REJECTED".equals(request.decision()))) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED",
                    "Validation failed", "decision must be APPROVED or REJECTED");
        }

        if ("REJECTED".equals(request.decision())
                && (request.rejectReason() == null || request.rejectReason().isBlank())) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED",
                    "Validation failed", "rejectReason is required when decision is REJECTED");
        }

        IdentityVerificationSubmission sub = submissionRepository.findById(submissionId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "SUBMISSION_NOT_FOUND",
                        "Submission not found", "submissionId does not exist"));

        if (!"PENDING_REVIEW".equals(sub.getReviewStatus())) {
            throw new ApiException(HttpStatus.CONFLICT, "SUBMISSION_NOT_PENDING",
                    "Submission is not pending review", "cannot review a submission that is not PENDING_REVIEW");
        }

        Instant now = Instant.now();
        sub.setReviewStatus(request.decision());
        sub.setReviewedAt(now);

        if ("REJECTED".equals(request.decision())) {
            sub.setRejectReason(request.rejectReason());
        }

        submissionRepository.save(sub);

        UserAccount account = userAccountRepository.findById(sub.getUserId()).orElseThrow();
        if ("APPROVED".equals(request.decision())) {
            account.setAuthenticationStatus("VERIFIED");
        } else {
            account.setAuthenticationStatus("REJECTED");
        }
        userAccountRepository.save(account);

        return new ReviewResponse(
                sub.getReviewStatus(),
                account.getAuthenticationStatus(),
                sub.getReviewedAt().toString(),
                sub.getRejectReason()
        );
    }

    record ReviewRequest(String decision, String rejectReason) {}

    record ReviewResponse(String reviewStatus, String authenticationStatus, String reviewedAt, String rejectReason) {}

    record PendingSubmissionItem(
            String submissionId, String userId, String realName, String studentNumber,
            String college, String major, String grade, String reviewStatus, String submittedAt,
            String materialAttachmentId, String materialContentType, Long materialSizeBytes
    ) {}

    java.io.InputStream getMaterialContent(UUID submissionId) {
        IdentityVerificationSubmission sub = submissionRepository.findById(submissionId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "SUBMISSION_NOT_FOUND",
                        "Submission not found", "submissionId does not exist"));
        if (sub.getMaterialAttachmentId() == null) {
            throw new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                    "No material attached", "submission has no material attachment");
        }
        IdentityMaterialAttachment attachment = attachmentRepository.findById(sub.getMaterialAttachmentId())
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                        "Attachment not found", "attachment metadata not found"));
        java.io.InputStream stream = objectStorageService.getObject(attachment.getObjectKey());
        if (stream == null) {
            throw new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                    "Attachment content not found", "object not found in storage");
        }
        return stream;
    }

    IdentityMaterialAttachment getMaterialMetadata(UUID submissionId) {
        IdentityVerificationSubmission sub = submissionRepository.findById(submissionId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "SUBMISSION_NOT_FOUND",
                        "Submission not found", "submissionId does not exist"));
        if (sub.getMaterialAttachmentId() == null) {
            throw new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                    "No material attached", "submission has no material attachment");
        }
        return attachmentRepository.findById(sub.getMaterialAttachmentId())
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "ATTACHMENT_NOT_FOUND",
                        "Attachment not found", "attachment metadata not found"));
    }
}
