package com.campusbuddy.auth;

import com.campusbuddy.common.ApiException;
import com.campusbuddy.config.CampusBuddyProperties;
import com.campusbuddy.security.JwtProperties;
import com.campusbuddy.security.JwtService;
import org.springframework.http.HttpStatus;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.Locale;
import java.util.Optional;
import java.util.UUID;
import java.util.regex.Pattern;

@Service
class AuthLoginService {

    private static final Pattern EMAIL_PATTERN = Pattern.compile("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$");

    private final UserAccountRepository userAccountRepository;
    private final JwtService jwtService;
    private final JwtProperties jwtProperties;
    private final BCryptPasswordEncoder passwordEncoder = new BCryptPasswordEncoder();

    AuthLoginService(UserAccountRepository userAccountRepository, JwtService jwtService, JwtProperties jwtProperties) {
        this.userAccountRepository = userAccountRepository;
        this.jwtService = jwtService;
        this.jwtProperties = jwtProperties;
    }

    @Transactional(readOnly = true)
    AuthLoginResponse login(AuthLoginRequest request) {
        String email = normalizeEmail(request == null ? null : request.campusEmail());
        String password = request == null ? null : request.password();
        validate(email, password);

        Optional<UserAccount> accountOpt = userAccountRepository.findByCampusEmail(email);
        if (accountOpt.isEmpty() || !passwordEncoder.matches(password, accountOpt.get().getPasswordHash())) {
            throw new ApiException(
                    HttpStatus.UNAUTHORIZED,
                    "INVALID_LOGIN_CREDENTIALS",
                    "Invalid login credentials",
                    "campusEmail or password is incorrect"
            );
        }

        UserAccount account = accountOpt.get();
        String accessToken = jwtService.issueAccessToken(
                account.getUserId(),
                account.getCampusEmail(),
                account.getDisplayName(),
                account.getAuthenticationStatus(),
                account.getAccountRole()
        );
        String refreshToken = jwtService.issueRefreshToken(account.getUserId());

        return new AuthLoginResponse(
                accessToken,
                (int) jwtProperties.getAccessTokenExpiresInSeconds(),
                refreshToken,
                (int) jwtProperties.getRefreshTokenExpiresInSeconds(),
                "Bearer",
                new AuthenticatedUserResponse(
                        account.getUserId().toString(),
                        account.getDisplayName(),
                        account.getAuthenticationStatus(),
                        account.getCampusEmailVerificationStatus()
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

    private String normalizeEmail(String email) {
        if (email == null) return null;
        return email.trim().toLowerCase(Locale.ROOT);
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
}
