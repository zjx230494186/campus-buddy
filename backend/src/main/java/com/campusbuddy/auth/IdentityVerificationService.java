package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

@Service
class IdentityVerificationService {

    private final IdentityVerificationSubmissionRepository submissionRepository;
    private final UserAccountRepository userAccountRepository;
    private final IdentityMaterialAttachmentRepository attachmentRepository;

    IdentityVerificationService(IdentityVerificationSubmissionRepository submissionRepository,
                                UserAccountRepository userAccountRepository,
                                IdentityMaterialAttachmentRepository attachmentRepository) {
        this.submissionRepository = submissionRepository;
        this.userAccountRepository = userAccountRepository;
        this.attachmentRepository = attachmentRepository;
    }

    @Transactional
    IdentityVerificationSubmitResponse submit(UUID userId, IdentityVerificationSubmitRequest request) {
        UserAccount account = userAccountRepository.findById(userId)
                .orElseThrow(() -> new ApiException(HttpStatus.UNAUTHORIZED, "UNAUTHORIZED", "User not found", "invalid userId in token"));

        String currentStatus = account.getAuthenticationStatus();

        if ("PENDING_REVIEW".equals(currentStatus)) {
            throw new ApiException(HttpStatus.CONFLICT, "IDENTITY_VERIFICATION_PENDING",
                    "Identity verification is pending review", "cannot resubmit while pending");
        }

        if ("VERIFIED".equals(currentStatus)) {
            throw new ApiException(HttpStatus.CONFLICT, "IDENTITY_ALREADY_VERIFIED",
                    "Identity already verified", "cannot resubmit after verification");
        }

        validate(request);

        UUID materialAttachmentId = null;
        if (request.materialAttachmentId() != null && !request.materialAttachmentId().isBlank()) {
            materialAttachmentId = validateAndResolveAttachment(userId, request.materialAttachmentId());
        }

        if (submissionRepository.existsByUserId(userId)) {
            IdentityVerificationSubmission existing = submissionRepository.findByUserId(userId).orElseThrow();
            existing.setReviewStatus("PENDING_REVIEW");
            existing.setRejectReason(null);
            existing.setReviewedAt(null);
            existing.setMaterialAttachmentId(materialAttachmentId);
            updateSubmissionFields(existing, request);
            submissionRepository.save(existing);
        } else {
            IdentityVerificationSubmission submission = new IdentityVerificationSubmission(
                    userId, request.realName(), request.studentNumber(),
                    request.college(), request.major(), request.grade(), Instant.now()
            );
            submission.setMaterialAttachmentId(materialAttachmentId);
            submissionRepository.save(submission);
        }

        account.setAuthenticationStatus("PENDING_REVIEW");
        userAccountRepository.save(account);

        IdentityVerificationSubmission saved = submissionRepository.findByUserId(userId).orElseThrow();
        return new IdentityVerificationSubmitResponse(
                "PENDING_REVIEW",
                saved.getSubmittedAt().toString(),
                saved.getRealName(),
                saved.getStudentNumber(),
                saved.getCollege(),
                saved.getMajor(),
                saved.getGrade()
        );
    }

    IdentityVerificationStatusResponse getStatus(UUID userId) {
        UserAccount account = userAccountRepository.findById(userId)
                .orElseThrow(() -> new ApiException(HttpStatus.UNAUTHORIZED, "UNAUTHORIZED", "User not found", "invalid userId in token"));

        String authenticationStatus = account.getAuthenticationStatus();
        List<String> allowedActions = computeAllowedActions(authenticationStatus);

        if (!submissionRepository.existsByUserId(userId)) {
            return new IdentityVerificationStatusResponse(
                    authenticationStatus, null, null, null, null,
                    null, null, null, null, null, allowedActions
            );
        }

        IdentityVerificationSubmission sub = submissionRepository.findByUserId(userId).orElseThrow();
        return new IdentityVerificationStatusResponse(
                authenticationStatus,
                sub.getReviewStatus(),
                sub.getSubmittedAt() != null ? sub.getSubmittedAt().toString() : null,
                sub.getReviewedAt() != null ? sub.getReviewedAt().toString() : null,
                sub.getRejectReason(),
                sub.getRealName(),
                sub.getStudentNumber(),
                sub.getCollege(),
                sub.getMajor(),
                sub.getGrade(),
                allowedActions
        );
    }

    private List<String> computeAllowedActions(String authenticationStatus) {
        List<String> actions = new ArrayList<>();
        switch (authenticationStatus) {
            case "UNVERIFIED", "REJECTED" -> actions.add("SUBMIT");
            case "PENDING_REVIEW" -> actions.add("RESUBMIT");
            case "VERIFIED" -> actions.add("VIEW");
        }
        return actions;
    }

    private void validate(IdentityVerificationSubmitRequest request) {
        if (request == null
                || request.realName() == null || request.realName().isBlank()
                || request.studentNumber() == null || request.studentNumber().isBlank()
                || request.college() == null || request.college().isBlank()
                || request.major() == null || request.major().isBlank()
                || request.grade() == null || request.grade().isBlank()) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED",
                    "Validation failed", "realName, studentNumber, college, major and grade are required");
        }
    }

    private void updateSubmissionFields(IdentityVerificationSubmission sub, IdentityVerificationSubmitRequest request) {
        sub.setRealName(request.realName());
        sub.setStudentNumber(request.studentNumber());
        sub.setCollege(request.college());
        sub.setMajor(request.major());
        sub.setGrade(request.grade());
        sub.setSubmittedAt(Instant.now());
    }

    private UUID validateAndResolveAttachment(UUID userId, String materialAttachmentIdStr) {
        UUID attachmentId;
        try {
            attachmentId = UUID.fromString(materialAttachmentIdStr);
        } catch (IllegalArgumentException e) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED",
                    "Validation failed", "materialAttachmentId is not a valid UUID");
        }
        IdentityMaterialAttachment attachment = attachmentRepository.findByAttachmentIdAndOwnerUserId(attachmentId, userId)
                .orElseThrow(() -> new ApiException(HttpStatus.FORBIDDEN, "ATTACHMENT_NOT_OWNED",
                        "Attachment not owned by current user", "materialAttachmentId does not belong to you"));
        if (!"ACTIVE".equals(attachment.getStatus()) || !"IDENTITY_MATERIAL".equals(attachment.getBusinessType())) {
            throw new ApiException(HttpStatus.CONFLICT, "ATTACHMENT_NOT_USABLE",
                    "Attachment not usable", "attachment is not active or not identity material type");
        }
        return attachmentId;
    }

    record IdentityVerificationSubmitRequest(
            String realName, String studentNumber, String college, String major, String grade,
            String materialAttachmentId
    ) {}

    record IdentityVerificationSubmitResponse(
            String authenticationStatus, String submittedAt,
            String realName, String studentNumber, String college, String major, String grade
    ) {}

    record IdentityVerificationStatusResponse(
            String authenticationStatus, String reviewStatus,
            String submittedAt, String reviewedAt, String rejectReason,
            String realName, String studentNumber, String college, String major, String grade,
            List<String> allowedActions
    ) {}
}
