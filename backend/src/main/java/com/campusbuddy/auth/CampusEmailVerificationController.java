package com.campusbuddy.auth;

import com.campusbuddy.auth.CampusEmailVerificationService.CampusEmailVerificationRequest;
import com.campusbuddy.auth.CampusEmailVerificationService.CampusEmailVerificationResponse;
import com.campusbuddy.auth.CampusEmailVerificationService.CampusEmailVerificationResult;
import com.campusbuddy.auth.CampusEmailVerificationService.CampusEmailVerificationSubmission;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/auth/campus-email")
class CampusEmailVerificationController {

    private final CampusEmailVerificationService verificationService;

    CampusEmailVerificationController(CampusEmailVerificationService verificationService) {
        this.verificationService = verificationService;
    }

    @PostMapping("/verification-codes")
    CampusEmailVerificationResponse sendCode(@RequestBody(required = false) CampusEmailVerificationRequest request) {
        return verificationService.sendCode(request);
    }

    @PostMapping("/verifications")
    CampusEmailVerificationResult verifyCode(@RequestBody(required = false) CampusEmailVerificationSubmission request) {
        return verificationService.verifyCode(request);
    }
}
