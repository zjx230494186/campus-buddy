package com.campusbuddy.auth;

import com.campusbuddy.auth.AuthRegistrationService.AuthRegistrationRequest;
import com.campusbuddy.auth.AuthRegistrationService.AuthRegistrationResponse;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/auth")
class AuthRegistrationController {

    private final AuthRegistrationService registrationService;

    AuthRegistrationController(AuthRegistrationService registrationService) {
        this.registrationService = registrationService;
    }

    @PostMapping("/register")
    AuthRegistrationResponse register(@RequestBody(required = false) AuthRegistrationRequest request) {
        return registrationService.register(request);
    }
}
