package com.campusbuddy.config;

import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertNotNull;

class DeployMultipartConfigurationTest {

    @Test
    void deployProfileAllowsDocumentedIdentityMaterialSize() throws IOException {
        Properties properties = new Properties();
        try (InputStream input = getClass().getResourceAsStream("/application-deploy.properties")) {
            assertNotNull(input, "application-deploy.properties must be available");
            properties.load(input);
        }

        assertEquals("10MB", properties.getProperty("spring.servlet.multipart.max-file-size"));
        assertEquals("12MB", properties.getProperty("spring.servlet.multipart.max-request-size"));
    }
}
