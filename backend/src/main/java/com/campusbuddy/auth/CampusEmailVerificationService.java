package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import com.campusbuddy.config.CampusBuddyProperties;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.ObjectProvider;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.Clock;
import java.time.Duration;
import java.time.Instant;
import java.util.HexFormat;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;

@Service
class CampusEmailVerificationService {

    static final int CODE_EXPIRES_IN_SECONDS = 600;
    static final int RESEND_AFTER_SECONDS = 60;
    private static final Set<String> SUPPORTED_PURPOSES = Set.of("REGISTER_OR_LOGIN");
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");

    private final CampusEmailVerificationCodeSender codeSender;
    private final Clock clock;
    private final Set<String> allowedDomains;
    private final int codeExpiresInSeconds;
    private final int resendAfterSeconds;
    private final SecureRandom secureRandom = new SecureRandom();
    private final Map<String, VerificationCodeRecord> records = new ConcurrentHashMap<>();
    private final Map<String, VerifiedTicketRecord> verifiedTickets = new ConcurrentHashMap<>();

    @Autowired
    CampusEmailVerificationService(
            CampusEmailVerificationCodeSender codeSender,
            ObjectProvider<Clock> clockProvider,
            CampusBuddyProperties campusBuddyProperties
    ) {
        this(codeSender, clockProvider.getIfAvailable(Clock::systemUTC), campusBuddyProperties);
    }

    CampusEmailVerificationService(CampusEmailVerificationCodeSender codeSender, Clock clock, CampusBuddyProperties campusBuddyProperties) {
        this.codeSender = codeSender;
        this.clock = clock;
        this.allowedDomains = campusBuddyProperties.getCampusEmail().getAllowedDomains();
        this.codeExpiresInSeconds = campusBuddyProperties.getCampusEmail().getCodeExpiresInSeconds();
        this.resendAfterSeconds = campusBuddyProperties.getCampusEmail().getResendAfterSeconds();
    }

    CampusEmailVerificationResponse sendCode(CampusEmailVerificationRequest request) {
        String email = normalizeEmail(request == null ? null : request.campusEmail());
        String purpose = normalizePurpose(request == null ? null : request.purpose());
        validate(email, purpose);

        Instant now = Instant.now(clock);
        String key = email + "|" + purpose;
        VerificationCodeRecord existing = records.get(key);
        if (existing != null && existing.nextAllowedAt().isAfter(now)) {
            long resendAfterSeconds = secondsUntil(now, existing.nextAllowedAt());
            throw new ApiException(
                    HttpStatus.TOO_MANY_REQUESTS,
                    "EMAIL_VERIFICATION_TOO_FREQUENT",
                    "Email verification request too frequent",
                    Map.of("resendAfterSeconds", resendAfterSeconds)
            );
        }

        String verificationCode = generateVerificationCode();
        Instant expiresAt = now.plusSeconds(codeExpiresInSeconds);
        Instant nextAllowedAt = now.plusSeconds(resendAfterSeconds);
        records.put(key, new VerificationCodeRecord(hashVerificationCode(email, purpose, verificationCode), expiresAt, nextAllowedAt));
        codeSender.send(email, verificationCode, purpose);

        return new CampusEmailVerificationResponse(
                maskEmail(email),
                "CODE_SENT",
                codeExpiresInSeconds,
                resendAfterSeconds
        );
    }

    CampusEmailVerificationResult verifyCode(CampusEmailVerificationSubmission request) {
        String email = normalizeEmail(request == null ? null : request.campusEmail());
        String purpose = normalizePurpose(request == null ? null : request.purpose());
        String code = normalizeCode(request == null ? null : request.code());
        validate(email, purpose, code);

        Instant now = Instant.now(clock);
        VerificationCodeRecord existing = records.get(email + "|" + purpose);
        if (existing == null) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "EMAIL_VERIFICATION_CODE_INVALID",
                    "Email verification code invalid",
                    "verification code does not match"
            );
        }

        if (now.isAfter(existing.expiresAt())) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "EMAIL_VERIFICATION_CODE_EXPIRED",
                    "Email verification code expired",
                    "verification code has expired"
            );
        }

        String requestCodeHash = hashVerificationCode(email, purpose, code);
        if (!MessageDigest.isEqual(
                existing.codeHash().getBytes(StandardCharsets.UTF_8),
                requestCodeHash.getBytes(StandardCharsets.UTF_8)
        )) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "EMAIL_VERIFICATION_CODE_INVALID",
                    "Email verification code invalid",
                    "verification code does not match"
            );
        }

        records.remove(email + "|" + purpose);
        String verificationTicket = generateVerificationTicket();
        verifiedTickets.put(
                email + "|" + purpose,
                new VerifiedTicketRecord(hashSensitiveValue(email, purpose, verificationTicket), existing.expiresAt())
        );
        return new CampusEmailVerificationResult(
                maskEmail(email),
                "VERIFIED",
                now.toString(),
                verificationTicket
        );
    }

    boolean consumeRegistrationTicket(String campusEmail, String verificationTicket) {
        String email = normalizeEmail(campusEmail);
        String ticket = verificationTicket == null ? null : verificationTicket.trim();
        if (email == null || ticket == null || ticket.isBlank()) {
            return false;
        }

        Instant now = Instant.now(clock);
        String key = email + "|REGISTER_OR_LOGIN";
        VerifiedTicketRecord existing = verifiedTickets.get(key);
        if (existing == null || now.isAfter(existing.expiresAt())) {
            verifiedTickets.remove(key);
            return false;
        }

        String requestTicketHash = hashSensitiveValue(email, "REGISTER_OR_LOGIN", ticket);
        if (!MessageDigest.isEqual(
                existing.ticketHash().getBytes(StandardCharsets.UTF_8),
                requestTicketHash.getBytes(StandardCharsets.UTF_8)
        )) {
            return false;
        }

        verifiedTickets.remove(key);
        return true;
    }

    private void validate(String email, String purpose) {
        if (email == null || purpose == null || !EMAIL_PATTERN.matcher(email).matches()
                || !SUPPORTED_PURPOSES.contains(purpose)) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "VALIDATION_FAILED",
                    "Validation failed",
                    "campusEmail and purpose are required"
            );
        }

        String domain = email.substring(email.indexOf('@') + 1);
        if (!allowedDomains.contains(domain)) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "INVALID_CAMPUS_EMAIL_DOMAIN",
                    "Invalid campus email domain",
                    "campusEmail domain is not allowed"
            );
        }
    }

    private void validate(String email, String purpose, String code) {
        validate(email, purpose);
        if (code == null || !code.matches("\\d{6}")) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "VALIDATION_FAILED",
                    "Validation failed",
                    "campusEmail, code and purpose are required"
            );
        }
    }

    private String normalizeEmail(String email) {
        if (email == null) {
            return null;
        }
        return email.trim().toLowerCase(Locale.ROOT);
    }

    private String normalizePurpose(String purpose) {
        if (purpose == null) {
            return null;
        }
        return purpose.trim().toUpperCase(Locale.ROOT);
    }

    private String normalizeCode(String code) {
        if (code == null) {
            return null;
        }
        return code.trim();
    }

    private String generateVerificationCode() {
        return "%06d".formatted(secureRandom.nextInt(1_000_000));
    }

    private String generateVerificationTicket() {
        return "cet_" + UUID.randomUUID();
    }

    private String hashVerificationCode(String email, String purpose, String verificationCode) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest((email + "|" + purpose + "|" + verificationCode)
                    .getBytes(StandardCharsets.UTF_8));
            return HexFormat.of().formatHex(hash);
        } catch (NoSuchAlgorithmException exception) {
            throw new IllegalStateException("SHA-256 is not available", exception);
        }
    }

    private String hashSensitiveValue(String email, String purpose, String value) {
        return hashVerificationCode(email, purpose, value);
    }

    private long secondsUntil(Instant now, Instant target) {
        return Math.max(1, Duration.between(now, target).toSeconds());
    }

    private String maskEmail(String email) {
        int atIndex = email.indexOf('@');
        String local = email.substring(0, atIndex);
        String domain = email.substring(atIndex);
        String prefix = local.length() <= 2 ? local.substring(0, 1) : local.substring(0, 2);
        return prefix + "***" + domain;
    }

    private record VerificationCodeRecord(String codeHash, Instant expiresAt, Instant nextAllowedAt) {
    }

    private record VerifiedTicketRecord(String ticketHash, Instant expiresAt) {
    }

    record CampusEmailVerificationRequest(String campusEmail, String purpose) {
    }

    record CampusEmailVerificationSubmission(String campusEmail, String code, String purpose) {
    }

    record CampusEmailVerificationResponse(
            String campusEmail,
            String verificationStatus,
            int expiresInSeconds,
            int resendAfterSeconds
    ) {
    }

    record CampusEmailVerificationResult(
            String campusEmail,
            String verificationStatus,
            String verifiedAt,
            String verificationTicket
    ) {
    }
}
