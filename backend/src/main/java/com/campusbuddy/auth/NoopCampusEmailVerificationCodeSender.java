package com.campusbuddy.auth;

import org.springframework.stereotype.Component;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;

@Component
@ConditionalOnProperty(prefix = "campus-buddy.campus-email", name = "delivery-mode", havingValue = "noop", matchIfMissing = true)
class NoopCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {

    @Override
    public void send(String campusEmail, String verificationCode, String purpose) {
        // Test-time placeholder: production email delivery is intentionally out of scope here.
    }
}
