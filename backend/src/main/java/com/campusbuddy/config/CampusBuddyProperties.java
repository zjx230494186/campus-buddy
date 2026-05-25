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
        private String deliveryMode = "noop";
        private Smtp smtp = new Smtp();

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

        public String getDeliveryMode() {
            return deliveryMode;
        }

        public void setDeliveryMode(String deliveryMode) {
            this.deliveryMode = deliveryMode;
        }

        public Smtp getSmtp() {
            return smtp;
        }

        public void setSmtp(Smtp smtp) {
            this.smtp = smtp;
        }

        public static class Smtp {
            private String host;
            private int port = 587;
            private String username;
            private String password;
            private String from;
            private String fromName = "校园搭子平台";
            private boolean auth = true;
            private boolean startTls = true;
            private boolean ssl = false;
            private int connectionTimeoutMillis = 5000;
            private int timeoutMillis = 5000;
            private int writeTimeoutMillis = 5000;

            public String getHost() {
                return host;
            }

            public void setHost(String host) {
                this.host = host;
            }

            public int getPort() {
                return port;
            }

            public void setPort(int port) {
                this.port = port;
            }

            public String getUsername() {
                return username;
            }

            public void setUsername(String username) {
                this.username = username;
            }

            public String getPassword() {
                return password;
            }

            public void setPassword(String password) {
                this.password = password;
            }

            public String getFrom() {
                return from;
            }

            public void setFrom(String from) {
                this.from = from;
            }

            public String getFromName() {
                return fromName;
            }

            public void setFromName(String fromName) {
                this.fromName = fromName;
            }

            public boolean isAuth() {
                return auth;
            }

            public void setAuth(boolean auth) {
                this.auth = auth;
            }

            public boolean isStartTls() {
                return startTls;
            }

            public void setStartTls(boolean startTls) {
                this.startTls = startTls;
            }

            public boolean isSsl() {
                return ssl;
            }

            public void setSsl(boolean ssl) {
                this.ssl = ssl;
            }

            public int getConnectionTimeoutMillis() {
                return connectionTimeoutMillis;
            }

            public void setConnectionTimeoutMillis(int connectionTimeoutMillis) {
                this.connectionTimeoutMillis = connectionTimeoutMillis;
            }

            public int getTimeoutMillis() {
                return timeoutMillis;
            }

            public void setTimeoutMillis(int timeoutMillis) {
                this.timeoutMillis = timeoutMillis;
            }

            public int getWriteTimeoutMillis() {
                return writeTimeoutMillis;
            }

            public void setWriteTimeoutMillis(int writeTimeoutMillis) {
                this.writeTimeoutMillis = writeTimeoutMillis;
            }
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
        private String accessKeyId;
        private String secretAccessKey;

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

        public String getAccessKeyId() {
            return accessKeyId;
        }

        public void setAccessKeyId(String accessKeyId) {
            this.accessKeyId = accessKeyId;
        }

        public String getSecretAccessKey() {
            return secretAccessKey;
        }

        public void setSecretAccessKey(String secretAccessKey) {
            this.secretAccessKey = secretAccessKey;
        }
    }
}
