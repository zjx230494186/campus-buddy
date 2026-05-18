package com.campusbuddy.security;

import org.springframework.boot.context.properties.ConfigurationProperties;

@ConfigurationProperties(prefix = "campus-buddy.security.jwt")
public class JwtProperties {

    private String secret = "default-dev-secret-key-must-be-at-least-32-chars-long-for-hmac-sha256";
    private long accessTokenExpiresInSeconds = 900;
    private long refreshTokenExpiresInSeconds = 2592000;

    public String getSecret() { return secret; }
    public void setSecret(String secret) { this.secret = secret; }
    public long getAccessTokenExpiresInSeconds() { return accessTokenExpiresInSeconds; }
    public void setAccessTokenExpiresInSeconds(long accessTokenExpiresInSeconds) { this.accessTokenExpiresInSeconds = accessTokenExpiresInSeconds; }
    public long getRefreshTokenExpiresInSeconds() { return refreshTokenExpiresInSeconds; }
    public void setRefreshTokenExpiresInSeconds(long refreshTokenExpiresInSeconds) { this.refreshTokenExpiresInSeconds = refreshTokenExpiresInSeconds; }
}
