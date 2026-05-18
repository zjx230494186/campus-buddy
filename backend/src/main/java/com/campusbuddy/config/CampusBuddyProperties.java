package com.campusbuddy.config;

import org.springframework.boot.context.properties.ConfigurationProperties;

import java.util.HashSet;
import java.util.Set;

@ConfigurationProperties(prefix = "campus-buddy")
public class CampusBuddyProperties {

    private CampusEmail campusEmail = new CampusEmail();
    private ObjectStorage objectStorage = new ObjectStorage();

    public CampusEmail getCampusEmail() {
        return campusEmail;
    }

    public void setCampusEmail(CampusEmail campusEmail) {
        this.campusEmail = campusEmail;
    }

    public ObjectStorage getObjectStorage() {
        return objectStorage;
    }

    public void setObjectStorage(ObjectStorage objectStorage) {
        this.objectStorage = objectStorage;
    }

    public static class CampusEmail {

        private Set<String> allowedDomains = new HashSet<>(Set.of("campus.edu.cn"));
        private int codeExpiresInSeconds = 600;
        private int resendAfterSeconds = 60;

        public Set<String> getAllowedDomains() {
            return allowedDomains;
        }

        public void setAllowedDomains(Set<String> allowedDomains) {
            this.allowedDomains = allowedDomains;
        }

        public int getCodeExpiresInSeconds() {
            return codeExpiresInSeconds;
        }

        public void setCodeExpiresInSeconds(int codeExpiresInSeconds) {
            this.codeExpiresInSeconds = codeExpiresInSeconds;
        }

        public int getResendAfterSeconds() {
            return resendAfterSeconds;
        }

        public void setResendAfterSeconds(int resendAfterSeconds) {
            this.resendAfterSeconds = resendAfterSeconds;
        }
    }

    public static class ObjectStorage {

        private String provider = "huaweicloud-obs";
        private String endpoint = "obs.cn-north-4.myhuaweicloud.com";
        private String region = "cn-north-4";
        private String bucket = "20260518-bighomework";
        private String accessMode = "backend-proxy";
        private boolean publicRead = false;
        private boolean corsEnabled = false;

        public String getProvider() {
            return provider;
        }

        public void setProvider(String provider) {
            this.provider = provider;
        }

        public String getEndpoint() {
            return endpoint;
        }

        public void setEndpoint(String endpoint) {
            this.endpoint = endpoint;
        }

        public String getRegion() {
            return region;
        }

        public void setRegion(String region) {
            this.region = region;
        }

        public String getBucket() {
            return bucket;
        }

        public void setBucket(String bucket) {
            this.bucket = bucket;
        }

        public String getAccessMode() {
            return accessMode;
        }

        public void setAccessMode(String accessMode) {
            this.accessMode = accessMode;
        }

        public boolean isPublicRead() {
            return publicRead;
        }

        public void setPublicRead(boolean publicRead) {
            this.publicRead = publicRead;
        }

        public boolean isCorsEnabled() {
            return corsEnabled;
        }

        public void setCorsEnabled(boolean corsEnabled) {
            this.corsEnabled = corsEnabled;
        }
    }
}
