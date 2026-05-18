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
@Table(name = "identity_verification_submission")
public class IdentityVerificationSubmission {

    @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private UUID submissionId;

    @Column(nullable = false)
    private UUID userId;

    @Column(nullable = false, length = 100)
    private String realName;

    @Column(nullable = false, length = 50)
    private String studentNumber;

    @Column(nullable = false, length = 100)
    private String college;

    @Column(nullable = false, length = 100)
    private String major;

    @Column(nullable = false, length = 20)
    private String grade;

    @Column(nullable = false, length = 30)
    private String reviewStatus = "PENDING_REVIEW";

    @Column(length = 500)
    private String rejectReason;

    @Column(nullable = false)
    private Instant submittedAt;

    @Column
    private Instant reviewedAt;

    @Column
    private UUID materialAttachmentId;

    protected IdentityVerificationSubmission() {
    }

    public IdentityVerificationSubmission(UUID userId, String realName, String studentNumber,
                                          String college, String major, String grade, Instant submittedAt) {
        this.userId = userId;
        this.realName = realName;
        this.studentNumber = studentNumber;
        this.college = college;
        this.major = major;
        this.grade = grade;
        this.submittedAt = submittedAt;
    }

    public UUID getSubmissionId() { return submissionId; }
    public UUID getUserId() { return userId; }
    public String getRealName() { return realName; }
    public String getStudentNumber() { return studentNumber; }
    public String getCollege() { return college; }
    public String getMajor() { return major; }
    public String getGrade() { return grade; }
    public String getReviewStatus() { return reviewStatus; }
    public String getRejectReason() { return rejectReason; }
    public Instant getSubmittedAt() { return submittedAt; }
    public Instant getReviewedAt() { return reviewedAt; }
    public UUID getMaterialAttachmentId() { return materialAttachmentId; }

    public void setReviewStatus(String reviewStatus) { this.reviewStatus = reviewStatus; }
    public void setRejectReason(String rejectReason) { this.rejectReason = rejectReason; }
    public void setReviewedAt(Instant reviewedAt) { this.reviewedAt = reviewedAt; }
    public void setRealName(String realName) { this.realName = realName; }
    public void setStudentNumber(String studentNumber) { this.studentNumber = studentNumber; }
    public void setCollege(String college) { this.college = college; }
    public void setMajor(String major) { this.major = major; }
    public void setGrade(String grade) { this.grade = grade; }
    public void setSubmittedAt(Instant submittedAt) { this.submittedAt = submittedAt; }
    public void setMaterialAttachmentId(UUID materialAttachmentId) { this.materialAttachmentId = materialAttachmentId; }
}
