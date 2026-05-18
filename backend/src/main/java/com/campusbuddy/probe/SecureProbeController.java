package com.campusbuddy.probe;

import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/probe")
public class SecureProbeController {

    @GetMapping("/secure")
    public SecureProbeResponse secure(Authentication authentication) {
        return new SecureProbeResponse(
                authentication != null && authentication.isAuthenticated(),
                authentication == null ? null : authentication.getName(),
                "jwt-placeholder"
        );
    }

    public record SecureProbeResponse(
            boolean authenticated,
            String principal,
            String authenticationMode
    ) {
    }
}
