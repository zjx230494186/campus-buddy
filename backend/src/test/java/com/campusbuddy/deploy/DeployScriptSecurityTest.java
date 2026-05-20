package com.campusbuddy.deploy;

import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import static org.junit.jupiter.api.Assertions.*;

class DeployScriptSecurityTest {

    private static final Path SCRIPT_PATH = Paths.get("../deploy/start_backend_deploy.sh");

    private String readScript() throws IOException {
        return Files.readString(SCRIPT_PATH);
    }

    @Test
    void scriptDoesNotContainDatasourcePasswordArg() throws IOException {
        String script = readScript();
        assertFalse(script.contains("--spring.datasource.password"),
            "start_backend_deploy.sh must not contain --spring.datasource.password on command line");
    }

    @Test
    void scriptDoesNotContainJwtSecretArg() throws IOException {
        String script = readScript();
        assertFalse(script.contains("--campus-buddy.security.jwt.secret"),
            "start_backend_deploy.sh must not contain --campus-buddy.security.jwt.secret on command line");
    }

    @Test
    void scriptDoesNotContainObsAccessKeyIdArg() throws IOException {
        String script = readScript();
        assertFalse(script.contains("--campus-buddy.object-storage.access-key-id"),
            "start_backend_deploy.sh must not contain OBS access-key-id on command line");
    }

    @Test
    void scriptDoesNotContainObsSecretAccessKeyArg() throws IOException {
        String script = readScript();
        assertFalse(script.contains("--campus-buddy.object-storage.secret-access-key"),
            "start_backend_deploy.sh must not contain OBS secret-access-key on command line");
    }
}
