package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import com.campusbuddy.config.CampusBuddyProperties;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.ObjectProvider;
import org.springframework.http.HttpStatus;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;

import java.time.Clock;
import java.time.Instant;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;

@Service
class AuthRegistrationService {

    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");

    private final CampusEmailVerificationService verificationService;
    private final Clock clock;
    private final Set<String> allowedDomains;
    private final BCryptPasswordEncoder passwordEncoder = new BCryptPasswordEncoder();
    private final Map<String, RegisteredAccount> accounts = new ConcurrentHashMap<>();

    @Autowired
    AuthRegistrationService(
            CampusEmailVerificationService verificationService,
            ObjectProvider<Clock> clockProvider,
            CampusBuddyProperties campusBuddyProperties
    ) {
        this(verificationService, clockProvider.getIfAvailable(Clock::systemUTC), campusBuddyProperties);
    }

    AuthRegistrationService(CampusEmailVerificationService verificationService, Clock clock, CampusBuddyProperties campusBuddyProperties) {
        this.verificationService = verificationService;
        this.clock = clock;
        this.allowedDomains = campusBuddyProperties.getCampusEmail().getAllowedDomains();
    }

    AuthRegistrationResponse register(AuthRegistrationRequest request) {
        String email = normalizeEmail(request == null ? null : request.campusEmail());
        String ticket = normalizeText(request == null ? null : request.verificationTicket());
        String password = request == null ? null : request.password();
        String displayName = normalizeText(request == null ? null : request.displayName());
        validate(email, ticket, password, displayName);

        if (!verificationService.consumeRegistrationTicket(email, ticket)) {
            throw new ApiException(
                    HttpStatus.CONFLICT,
                    "CAMPUS_EMAIL_NOT_VERIFIED",
                    "Campus email not verified",
                    "verificationTicket is invalid or expired"
            );
        }

        Instant createdAt = Instant.now(clock);
        RegisteredAccount account = new RegisteredAccount(
                UUID.randomUUID().toString(),
                email,
                passwordEncoder.encode(password),
                displayName,
                createdAt
        );
        RegisteredAccount existing = accounts.putIfAbsent(email, account);
        if (existing != null) {
            throw new ApiException(
                    HttpStatus.CONFLICT,
                    "EMAIL_ALREADY_REGISTERED",
                    "Email already registered",
                    "campusEmail already exists"
            );
        }

        return new AuthRegistrationResponse(
                account.userId(),
                maskEmail(account.campusEmail()),
                account.displayName(),
                "UNVERIFIED",
                "VERIFIED",
                account.createdAt().toString()
        );
    }

    Optional<AuthenticatedAccount> authenticateForLogin(String campusEmail, String password) {
        String email = normalizeEmail(campusEmail);
        if (email == null || password == null) {
            return Optional.empty();
        }

        RegisteredAccount account = accounts.get(email);
        if (account == null || !passwordEncoder.matches(password, account.passwordHash())) {
            return Optional.empty();
        }

        return Optional.of(new AuthenticatedAccount(
                account.userId(),
                account.campusEmail(),
                account.displayName(),
                "UNVERIFIED",
                "VERIFIED"
        ));
    }

    private void validate(String email, String ticket, String password, String displayName) {
        if (email == null || ticket == null || ticket.isBlank()
                || password == null || password.length() < 8
                || displayName == null || displayName.isBlank()
                || !EMAIL_PATTERN.matcher(email).matches()) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "VALIDATION_FAILED",
                    "Validation failed",
                    "campusEmail, verificationTicket, password and displayName are required"
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

    private String normalizeEmail(String email) {
        if (email == null) {
            return null;
        }
        return email.trim().toLowerCase(Locale.ROOT);
    }

    private String normalizeText(String value) {
        if (value == null) {
            return null;
        }
        return value.trim();
    }

    private String maskEmail(String email) {
        int atIndex = email.indexOf('@');
        String local = email.substring(0, atIndex);
        String domain = email.substring(atIndex);
        String prefix = local.length() <= 2 ? local.substring(0, 1) : local.substring(0, 2);
        return prefix + "***" + domain;
    }

    record AuthRegistrationRequest(
            String campusEmail,
            String verificationTicket,
            String password,
            String displayName,
            String college,
            String grade
    ) {
    }

    record AuthRegistrationResponse(
            String userId,
            String campusEmail,
            String displayName,
            String authenticationStatus,
            String campusEmailVerificationStatus,
            String createdAt
    ) {
    }

    private record RegisteredAccount(
            String userId,
            String campusEmail,
            String passwordHash,
            String displayName,
            Instant createdAt
    ) {
    }

    record AuthenticatedAccount(
            String userId,
            String campusEmail,
            String displayName,
            String authenticationStatus,
            String campusEmailVerificationStatus
    ) {
    }
}
