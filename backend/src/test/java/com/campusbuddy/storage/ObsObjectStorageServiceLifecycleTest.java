package com.campusbuddy.storage;

import com.campusbuddy.config.CampusBuddyProperties;
import com.campusbuddy.config.ObjectStorageConfiguration;
import org.junit.jupiter.api.Test;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Primary;

import java.io.IOException;

import static org.junit.jupiter.api.Assertions.*;

class ObsObjectStorageServiceLifecycleTest {

    @Test
    void closeIsCalledOnDestroyMethod() throws IOException {
        CampusBuddyProperties.ObjectStorage config = new CampusBuddyProperties.ObjectStorage();
        config.setAccessKeyId("fake-ak-for-lifecycle-test");
        config.setSecretAccessKey("fake-sk-for-lifecycle-test");

        ObsObjectStorageService service = new ObsObjectStorageService(config);
        assertFalse(service.isClosed());
        service.close();
        assertTrue(service.isClosed());
    }

    @Test
    void springDestroyMethodInvokesClose() {
        CampusBuddyProperties props = new CampusBuddyProperties();
        props.getObjectStorage().setAccessKeyId("fake-ak-for-lifecycle-test");
        props.getObjectStorage().setSecretAccessKey("fake-sk-for-lifecycle-test");

        try (AnnotationConfigApplicationContext ctx = new AnnotationConfigApplicationContext()) {
            ctx.registerBean(CampusBuddyProperties.class, () -> props);
            ctx.register(TestObjectStorageConfig.class);
            ctx.refresh();

            ObsObjectStorageService service = ctx.getBean(ObsObjectStorageService.class);
            assertFalse(service.isClosed());

            ctx.close();
            assertTrue(service.isClosed());
        }
    }

    @Configuration
    static class TestObjectStorageConfig {
        @Bean(destroyMethod = "close")
        ObsObjectStorageService obsObjectStorageService(CampusBuddyProperties campusBuddyProperties) {
            return new ObsObjectStorageService(campusBuddyProperties.getObjectStorage());
        }
    }
}
