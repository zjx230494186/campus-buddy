package com.campusbuddy.security;

import com.campusbuddy.TestcontainersConfiguration;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.context.annotation.Import;
import org.springframework.test.web.servlet.MockMvc;

import java.util.UUID;

import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.emptyOrNullString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class SecurityProbeEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private JwtService jwtService;

    @Test
    void publicHealthEndpointAllowsAnonymousAccess() throws Exception {
        mockMvc.perform(get("/api/health"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.status").value("UP"));
    }

    @Test
    void secureProbeEndpointRejectsRequestWithoutToken() throws Exception {
        mockMvc.perform(get("/api/probe/secure"))
                .andExpect(status().isUnauthorized())
                .andExpect(jsonPath("$.code").value("UNAUTHORIZED"))
                .andExpect(jsonPath("$.message").value("Authentication required"))
                .andExpect(jsonPath("$.details").value("Missing or invalid bearer token"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }

    @Test
    void secureProbeEndpointAllowsRequestWithValidJwt() throws Exception {
        String accessToken = jwtService.issueAccessToken(
                UUID.randomUUID(), "test@campus.edu.cn", "TestUser", "UNVERIFIED"
        );

        mockMvc.perform(get("/api/probe/secure")
                        .header("Authorization", "Bearer " + accessToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticated").value(true));
    }
}
