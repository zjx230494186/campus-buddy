package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.time.Instant;
import java.util.HexFormat;
import java.util.Locale;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Pattern;

@Service
class AuthLoginService {

    private static final int ACCESS_TOKEN_EXPIRES_IN_SECONDS = 900;
    private static final int REFRESH_TOKEN_EXPIRES_IN_SECONDS = 2_592_000;
    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");

    private final AuthRegistrationService registrationService;
    private final Map<String, RefreshSession> refreshSessions = new ConcurrentHashMap<>();

    AuthLoginService(AuthRegistrationService registrationService) {
        this.registrationService = registrationService;
    }

    AuthLoginResponse login(AuthLoginRequest request) {
        String email = normalizeEmail(request == null ? null : request.campusEmail());
        String password = request == null ? null : request.password();
        validate(email, password);

        AuthRegistrationService.AuthenticatedAccount account = registrationService
                .authenticateForLogin(email, password)
                .orElseThrow(this::invalidCredentials);

        String accessToken = generateToken("cat");
        String refreshToken = generateToken("crt");
        refreshSessions.put(
                hashToken(refreshToken),
                new RefreshSession(account.userId(), "ACTIVE", Instant.now().plusSeconds(REFRESH_TOKEN_EXPIRES_IN_SECONDS))
        );

        return new AuthLoginResponse(
                accessToken,
                ACCESS_TOKEN_EXPIRES_IN_SECONDS,
                refreshToken,
                REFRESH_TOKEN_EXPIRES_IN_SECONDS,
                "Bearer",
                new AuthenticatedUserResponse(
                        account.userId(),
                        account.displayName(),
                        account.authenticationStatus(),
                        account.campusEmailVerificationStatus()
                )
        );
    }

    private void validate(String email, String password) {
        if (email == null || !EMAIL_PATTERN.matcher(email).matches()
                || password == null || password.isBlank()) {
            throw new ApiException(
                    HttpStatus.BAD_REQUEST,
                    "VALIDATION_FAILED",
                    "Validation failed",
                    "campusEmail and password are required"
            );
        }
    }

    private ApiException invalidCredentials() {
        return new ApiException(
                HttpStatus.UNAUTHORIZED,
                "INVALID_LOGIN_CREDENTIALS",
                "Invalid login credentials",
                "campusEmail or password is incorrect"
        );
    }

    private String normalizeEmail(String email) {
        if (email == null) {
            return null;
        }
        return email.trim().toLowerCase(Locale.ROOT);
    }

    private String generateToken(String prefix) {
        return prefix + "_" + UUID.randomUUID();
    }

    private String hashToken(String token) {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            byte[] hash = digest.digest(token.getBytes(StandardCharsets.UTF_8));
            return HexFormat.of().formatHex(hash);
        } catch (NoSuchAlgorithmException exception) {
            throw new IllegalStateException("SHA-256 is not available", exception);
        }
    }

    record AuthLoginRequest(
            String campusEmail,
            String password,
            String clientName,
            String deviceId
    ) {
    }

    record AuthLoginResponse(
            String accessToken,
            int accessTokenExpiresInSeconds,
            String refreshToken,
            int refreshTokenExpiresInSeconds,
            String tokenType,
            AuthenticatedUserResponse user
    ) {
    }

    record AuthenticatedUserResponse(
            String userId,
            String displayName,
            String authenticationStatus,
            String campusEmailVerificationStatus
    ) {
    }

    private record RefreshSession(String userId, String status, Instant expiresAt) {
    }
}
