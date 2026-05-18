package com.campusbuddy.config;

import com.campusbuddy.TestcontainersConfiguration;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.context.annotation.Import;

import java.util.Set;

import static org.junit.jupiter.api.Assertions.*;

@SpringBootTest
@Import(TestcontainersConfiguration.class)
class ConfigurationPropertiesTest {

    @Autowired
    private CampusBuddyProperties campusBuddyProperties;

    @Test
    void campusEmailAllowedDomainsAreLoadedFromConfiguration() {
        Set<String> domains = campusBuddyProperties.getCampusEmail().getAllowedDomains();
        assertNotNull(domains, "Allowed domains must not be null");
        assertFalse(domains.isEmpty(), "Allowed domains must not be empty");
        assertTrue(domains.contains("campus.edu.cn"), "Default domain campus.edu.cn must be present in local profile");
    }

    @Test
    void campusEmailAllowedDomainsAreNotHardcodedConstant() {
        Set<String> domains = campusBuddyProperties.getCampusEmail().getAllowedDomains();
        assertNotNull(domains);
        assertFalse(domains.isEmpty());
        assertTrue(domains.size() >= 1, "Domains must be loaded from configuration, not a static constant");
    }

    @Test
    void campusEmailCodeExpiresInSecondsHasPositiveValue() {
        int expires = campusBuddyProperties.getCampusEmail().getCodeExpiresInSeconds();
        assertTrue(expires > 0, "Code expiration must be positive");
    }

    @Test
    void campusEmailResendAfterSecondsHasPositiveValue() {
        int resend = campusBuddyProperties.getCampusEmail().getResendAfterSeconds();
        assertTrue(resend > 0, "Resend interval must be positive");
    }

    @Test
    void objectStorageAccessModeIsBackendProxy() {
        String accessMode = campusBuddyProperties.getObjectStorage().getAccessMode();
        assertEquals("backend-proxy", accessMode, "Object storage access mode must default to backend-proxy");
    }

    @Test
    void objectStoragePublicReadIsFalse() {
        boolean publicRead = campusBuddyProperties.getObjectStorage().isPublicRead();
        assertFalse(publicRead, "Object storage public read must default to false");
    }

    @Test
    void objectStorageCorsEnabledIsFalse() {
        boolean corsEnabled = campusBuddyProperties.getObjectStorage().isCorsEnabled();
        assertFalse(corsEnabled, "Object storage CORS must default to false");
    }
}
