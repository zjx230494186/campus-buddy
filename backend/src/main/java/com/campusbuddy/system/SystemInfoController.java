package com.campusbuddy.system;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
@RequestMapping("/api/system")
public class SystemInfoController {

    private final String version;

    public SystemInfoController(
            @Value("${campus-buddy.system.version:0.0.1-SNAPSHOT}") String version
    ) {
        this.version = version;
    }

    @GetMapping("/info")
    public SystemInfoResponse info() {
        return new SystemInfoResponse(
                "campus-buddy-backend",
                version,
                true
        );
    }

    public record SystemInfoResponse(
            String serviceName,
            String version,
            boolean technicalSpike
    ) {
    }
}
