package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import com.campusbuddy.config.CampusBuddyProperties;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.ObjectProvider;
import org.springframework.http.HttpStatus;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Clock;
import java.time.Instant;
import java.util.Locale;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.regex.Pattern;

@Service
class AuthRegistrationService {

    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");

    private final CampusEmailVerificationService verificationService;
    private final UserAccountRepository userAccountRepository;
    private final Clock clock;
    private final Set<String> allowedDomains;
    private final BCryptPasswordEncoder passwordEncoder = new BCryptPasswordEncoder();

    @Autowired
    AuthRegistrationService(
            CampusEmailVerificationService verificationService,
            UserAccountRepository userAccountRepository,
            ObjectProvider<Clock> clockProvider,
            CampusBuddyProperties campusBuddyProperties
    ) {
        this(verificationService, userAccountRepository, clockProvider.getIfAvailable(Clock::systemUTC), campusBuddyProperties);
    }

    AuthRegistrationService(CampusEmailVerificationService verificationService, UserAccountRepository userAccountRepository, Clock clock, CampusBuddyProperties campusBuddyProperties) {
        this.verificationService = verificationService;
        this.userAccountRepository = userAccountRepository;
        this.clock = clock;
        this.allowedDomains = campusBuddyProperties.getCampusEmail().getAllowedDomains();
    }

    @Transactional
    AuthRegistrationResponse register(AuthRegistrationRequest request) {
        String email = normalizeEmail(request == null ? null : request.campusEmail());
        String ticket = normalizeText(request == null ? null : request.verificationTicket());
        String password = request == null ? null : request.password();
        String displayName = normalizeText(request == null ? null : request.displayName());
        validate(email, ticket, password, displayName);

        if (userAccountRepository.existsByCampusEmail(email)) {
            throw new ApiException(
                    HttpStatus.CONFLICT,
                    "EMAIL_ALREADY_REGISTERED",
                    "Email already registered",
                    "campusEmail already exists"
            );
        }

        if (!verificationService.consumeRegistrationTicket(email, ticket)) {
            throw new ApiException(
                    HttpStatus.CONFLICT,
                    "CAMPUS_EMAIL_NOT_VERIFIED",
                    "Campus email not verified",
                    "verificationTicket is invalid or expired"
            );
        }

        Instant createdAt = Instant.now(clock);
        UserAccount account = new UserAccount(email, passwordEncoder.encode(password), displayName, createdAt);
        account.setCampusEmailVerificationStatus("VERIFIED");
        userAccountRepository.save(account);

        return new AuthRegistrationResponse(
                account.getUserId().toString(),
                maskEmail(account.getCampusEmail()),
                account.getDisplayName(),
                account.getAuthenticationStatus(),
                account.getCampusEmailVerificationStatus(),
                account.getCreatedAt().toString()
        );
    }

    Optional<AuthenticatedAccount> authenticateForLogin(String campusEmail, String password) {
        String email = normalizeEmail(campusEmail);
        if (email == null || password == null) {
            return Optional.empty();
        }

        Optional<UserAccount> accountOpt = userAccountRepository.findByCampusEmail(email);
        if (accountOpt.isEmpty() || !passwordEncoder.matches(password, accountOpt.get().getPasswordHash())) {
            return Optional.empty();
        }

        UserAccount account = accountOpt.get();
        return Optional.of(new AuthenticatedAccount(
                account.getUserId().toString(),
                account.getCampusEmail(),
                account.getDisplayName(),
                account.getAuthenticationStatus(),
                account.getCampusEmailVerificationStatus()
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
        if (email == null) return null;
        return email.trim().toLowerCase(Locale.ROOT);
    }

    private String normalizeText(String value) {
        if (value == null) return null;
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

    record AuthenticatedAccount(
            String userId,
            String campusEmail,
            String displayName,
            String authenticationStatus,
            String campusEmailVerificationStatus
    ) {
    }
}
