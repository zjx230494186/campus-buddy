package com.campusbuddy;

import com.campusbuddy.config.CampusBuddyProperties;
import com.campusbuddy.security.JwtProperties;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.context.properties.EnableConfigurationProperties;

@SpringBootApplication
@EnableConfigurationProperties({CampusBuddyProperties.class, JwtProperties.class})
public class CampusBuddyBackendApplication {

    public static void main(String[] args) {
        SpringApplication.run(CampusBuddyBackendApplication.class, args);
    }
}
