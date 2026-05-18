package com.campusbuddy.auth;

import org.springframework.stereotype.Component;

@Component
class NoopCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {

    @Override
    public void send(String campusEmail, String verificationCode, String purpose) {
        // Test-time placeholder: production email delivery is intentionally out of scope here.
    }
}
