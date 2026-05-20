package com.campusbuddy.deploy;

import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.*;

class SystemdServiceSecurityTest {

    private static final Path UNIT_PATH = Paths.get("../deploy/campus-buddy-backend.service");
    private static final Path WRAPPER_PATH = Paths.get("../deploy/start_backend_service.sh");

    private String readFile(Path path) throws IOException {
        return Files.readString(path);
    }

    @Test
    void unitFileDoesNotContainDatasourcePassword() throws IOException {
        String content = readFile(UNIT_PATH);
        assertFalse(content.contains("--spring.datasource.password"),
            "systemd unit must not contain --spring.datasource.password");
    }

    @Test
    void unitFileDoesNotContainJwtSecret() throws IOException {
        String content = readFile(UNIT_PATH);
        assertFalse(content.contains("--campus-buddy.security.jwt.secret"),
            "systemd unit must not contain --campus-buddy.security.jwt.secret");
    }

    @Test
    void unitFileDoesNotContainObsCredentials() throws IOException {
        String content = readFile(UNIT_PATH);
        assertFalse(content.contains("--campus-buddy.object-storage.access-key-id"),
            "systemd unit must not contain OBS access-key-id");
        assertFalse(content.contains("--campus-buddy.object-storage.secret-access-key"),
            "systemd unit must not contain OBS secret-access-key");
    }

    @Test
    void unitFileReferencesEnvFile() throws IOException {
        String content = readFile(UNIT_PATH);
        assertTrue(content.contains("start_backend_service.sh"),
            "unit must reference the wrapper script");
    }

    @Test
    void wrapperScriptDoesNotContainSensitiveArgs() throws IOException {
        String content = readFile(WRAPPER_PATH);
        assertFalse(content.contains("--spring.datasource.password"),
            "wrapper must not contain --spring.datasource.password");
        assertFalse(content.contains("--campus-buddy.security.jwt.secret"),
            "wrapper must not contain --campus-buddy.security.jwt.secret");
        assertFalse(content.contains("--campus-buddy.object-storage.access-key-id"),
            "wrapper must not contain OBS access-key-id");
        assertFalse(content.contains("--campus-buddy.object-storage.secret-access-key"),
            "wrapper must not contain OBS secret-access-key");
    }

    @Test
    void wrapperScriptSourcesBackendEnv() throws IOException {
        String content = readFile(WRAPPER_PATH);
        assertTrue(content.contains("/etc/campus-buddy/backend.env"),
            "wrapper must source /etc/campus-buddy/backend.env");
    }

    @Test
    void wrapperScriptMapsObsVariableNames() throws IOException {
        String content = readFile(WRAPPER_PATH);
        assertTrue(content.contains("OBS_ACCESS_KEY_ID") && content.contains("OBJECT_STORAGE_ACCESS_KEY_ID"),
            "wrapper must map OBJECT_STORAGE_ACCESS_KEY_ID to OBS_ACCESS_KEY_ID");
        assertTrue(content.contains("OBS_SECRET_ACCESS_KEY") && content.contains("OBJECT_STORAGE_SECRET_ACCESS_KEY"),
            "wrapper must map OBJECT_STORAGE_SECRET_ACCESS_KEY to OBS_SECRET_ACCESS_KEY");
    }
}
