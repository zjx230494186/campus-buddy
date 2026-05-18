package com.campusbuddy.security;

import org.springframework.boot.context.properties.ConfigurationProperties;

@ConfigurationProperties(prefix = "campus-buddy.security.jwt-placeholder")
public class JwtPlaceholderProperties {

    private String testToken = "technical-spike-test-token";
    private String principal = "technical-spike-user";

    public String getTestToken() {
        return testToken;
    }

    public void setTestToken(String testToken) {
        this.testToken = testToken;
    }

    public String getPrincipal() {
        return principal;
    }

    public void setPrincipal(String principal) {
        this.principal = principal;
    }
}
