package com.campusbuddy.config;

import com.campusbuddy.storage.InMemoryObjectStorageService;
import com.campusbuddy.storage.ObjectStorageService;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Profile;

@Configuration
public class ObjectStorageConfiguration {

    @Bean
    @Profile({"test", "local", "local-h2", "deploy"})
    ObjectStorageService inMemoryObjectStorageService() {
        return new InMemoryObjectStorageService();
    }
}
