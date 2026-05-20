package com.campusbuddy.storage;

import com.campusbuddy.config.CampusBuddyProperties;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.io.TempDir;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.MessageDigest;
import java.util.HexFormat;

import static org.junit.jupiter.api.Assertions.*;

class ObsObjectStorageServiceTest {

    private CampusBuddyProperties.ObjectStorage testConfig(String ak, String sk) {
        CampusBuddyProperties.ObjectStorage config = new CampusBuddyProperties.ObjectStorage();
        config.setAccessKeyId(ak);
        config.setSecretAccessKey(sk);
        return config;
    }

    @Test
    void constructorRejectsMissingAccessKeyId() {
        CampusBuddyProperties.ObjectStorage config = testConfig(null, "some-secret");
        IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
            () -> new ObsObjectStorageService(config));
        assertTrue(ex.getMessage().contains("accessKeyId"));
    }

    @Test
    void constructorRejectsBlankAccessKeyId() {
        CampusBuddyProperties.ObjectStorage config = testConfig("  ", "some-secret");
        IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
            () -> new ObsObjectStorageService(config));
        assertTrue(ex.getMessage().contains("accessKeyId"));
    }

    @Test
    void constructorRejectsMissingSecretAccessKey() {
        CampusBuddyProperties.ObjectStorage config = testConfig("some-ak", null);
        IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
            () -> new ObsObjectStorageService(config));
        assertTrue(ex.getMessage().contains("secretAccessKey"));
    }

    @Test
    void constructorRejectsBlankSecretAccessKey() {
        CampusBuddyProperties.ObjectStorage config = testConfig("some-ak", "  ");
        IllegalArgumentException ex = assertThrows(IllegalArgumentException.class,
            () -> new ObsObjectStorageService(config));
        assertTrue(ex.getMessage().contains("secretAccessKey"));
    }

    @Test
    void inMemoryDeleteObjectRemovesKey() {
        InMemoryObjectStorageService service = new InMemoryObjectStorageService();
        byte[] data = "hello".getBytes(StandardCharsets.UTF_8);
        service.putObject("test-key", "text/plain", data);
        assertNotNull(service.getObject("test-key"));
        service.deleteObject("test-key");
        assertNull(service.getObject("test-key"));
    }

    @Test
    void inMemoryGetObjectReturnsNullForMissingKey() {
        InMemoryObjectStorageService service = new InMemoryObjectStorageService();
        assertNull(service.getObject("nonexistent"));
    }
}
