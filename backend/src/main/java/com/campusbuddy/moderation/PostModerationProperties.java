package com.campusbuddy.moderation;

import org.springframework.boot.context.properties.ConfigurationProperties;

@ConfigurationProperties(prefix = "campus-buddy.post-moderation")
public class PostModerationProperties {

    private boolean enabled = false;
    private String provider = "noop";
    private String apiKey;
    private String model = "gpt-4o-mini";
    private String baseUrl = "https://api.openai.com/v1/chat/completions";
    private int timeoutMillis = 8000;
    private double autoApproveThreshold = 0.92;
    private double autoRejectThreshold = 0.85;
    private int maxInputChars = 2000;

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(boolean enabled) {
        this.enabled = enabled;
    }

    public String getProvider() {
        return provider;
    }

    public void setProvider(String provider) {
        this.provider = provider;
    }

    public String getApiKey() {
        return apiKey;
    }

    public void setApiKey(String apiKey) {
        this.apiKey = apiKey;
    }

    public String getModel() {
        return model;
    }

    public void setModel(String model) {
        this.model = model;
    }

    public String getBaseUrl() {
        return baseUrl;
    }

    public void setBaseUrl(String baseUrl) {
        this.baseUrl = baseUrl;
    }

    public int getTimeoutMillis() {
        return timeoutMillis;
    }

    public void setTimeoutMillis(int timeoutMillis) {
        this.timeoutMillis = timeoutMillis;
    }

    public double getAutoApproveThreshold() {
        return autoApproveThreshold;
    }

    public void setAutoApproveThreshold(double autoApproveThreshold) {
        this.autoApproveThreshold = autoApproveThreshold;
    }

    public double getAutoRejectThreshold() {
        return autoRejectThreshold;
    }

    public void setAutoRejectThreshold(double autoRejectThreshold) {
        this.autoRejectThreshold = autoRejectThreshold;
    }

    public int getMaxInputChars() {
        return maxInputChars;
    }

    public void setMaxInputChars(int maxInputChars) {
        this.maxInputChars = maxInputChars;
    }
}
