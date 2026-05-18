package com.campusbuddy.auth;

import com.campusbuddy.auth.AuthLoginService.AuthLoginRequest;
import com.campusbuddy.auth.AuthLoginService.AuthLoginResponse;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/auth")
class AuthLoginController {

    private final AuthLoginService loginService;

    AuthLoginController(AuthLoginService loginService) {
        this.loginService = loginService;
    }

    @PostMapping("/login")
    AuthLoginResponse login(@RequestBody(required = false) AuthLoginRequest request) {
        return loginService.login(request);
    }
}
