package com.campusbuddy.security;

import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.test.web.servlet.MockMvc;

import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.emptyOrNullString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@SpringBootTest(properties = "spring.autoconfigure.exclude=org.springframework.boot.jdbc.autoconfigure.DataSourceAutoConfiguration")
@AutoConfigureMockMvc
class SecurityProbeEndpointTest {

    private static final String TEST_TOKEN = "technical-spike-test-token";

    @Autowired
    private MockMvc mockMvc;

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
    void secureProbeEndpointAllowsRequestWithTestToken() throws Exception {
        mockMvc.perform(get("/api/probe/secure")
                        .header("Authorization", "Bearer " + TEST_TOKEN))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.authenticated").value(true))
                .andExpect(jsonPath("$.principal").value("technical-spike-user"))
                .andExpect(jsonPath("$.authenticationMode").value("jwt-placeholder"));
    }
}
