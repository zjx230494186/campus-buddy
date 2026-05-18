package com.campusbuddy.system;

import com.campusbuddy.TestcontainersConfiguration;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.context.annotation.Import;
import org.springframework.test.web.servlet.MockMvc;

import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.emptyOrNullString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class SystemInfoEndpointTest {

    @Autowired
    private MockMvc mockMvc;

    @Test
    void getSystemInfoReturnsBasicRestJsonContract() throws Exception {
        mockMvc.perform(get("/api/system/info"))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.serviceName").value("campus-buddy-backend"))
                .andExpect(jsonPath("$.version").value("0.0.1-SNAPSHOT"))
                .andExpect(jsonPath("$.technicalSpike").value(true));
    }

    @Test
    void unknownApiPathReturnsUnifiedErrorResponse() throws Exception {
        mockMvc.perform(get("/api/unknown-resource"))
                .andExpect(status().isNotFound())
                .andExpect(jsonPath("$.code").value("RESOURCE_NOT_FOUND"))
                .andExpect(jsonPath("$.message").value("Resource not found"))
                .andExpect(jsonPath("$.details").value("/api/unknown-resource"))
                .andExpect(jsonPath("$.traceId", not(emptyOrNullString())));
    }
}
