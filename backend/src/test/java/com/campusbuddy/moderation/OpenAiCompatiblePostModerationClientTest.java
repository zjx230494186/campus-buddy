package com.campusbuddy.moderation;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpServer;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.Test;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicReference;

import static org.junit.jupiter.api.Assertions.*;

class OpenAiCompatiblePostModerationClientTest {

    private HttpServer server;

    @AfterEach
    void stopServer() {
        if (server != null) {
            server.stop(0);
        }
    }

    @Test
    void parsesJsonDecisionFromOpenAiCompatibleResponse() throws Exception {
        AtomicReference<String> authorization = new AtomicReference<>();
        AtomicReference<String> body = new AtomicReference<>();
        server = startServer(authorization, body, """
                {"choices":[{"message":{"content":"{\\"decision\\":\\"REJECT\\",\\"confidence\\":0.93,\\"reason\\":\\"unsafe\\",\\"userVisibleReason\\":\\"内容不符合平台发布规范，请修改后重新提交。\\",\\"category\\":\\"SAFETY\\"}"}}]}
                """);

        PostModerationProperties properties = new PostModerationProperties();
        properties.setBaseUrl("http://127.0.0.1:" + server.getAddress().getPort() + "/v1/chat/completions");
        properties.setApiKey("test-api-key");
        properties.setModel("test-model");
        OpenAiCompatiblePostModerationClient client = new OpenAiCompatiblePostModerationClient(properties, new ObjectMapper());

        PostModerationDecision decision = client.moderate(sampleRequest());

        assertEquals("Bearer test-api-key", authorization.get());
        assertTrue(body.get().contains("\"model\":\"test-model\""));
        assertEquals("REJECT", decision.decision());
        assertEquals(0.93, decision.confidence());
        assertEquals("unsafe", decision.reason());
        assertEquals("内容不符合平台发布规范，请修改后重新提交。", decision.userVisibleReason());
        assertEquals("SAFETY", decision.category());
    }

    @Test
    void missingApiKeyFailsBeforeProviderCall() {
        PostModerationProperties properties = new PostModerationProperties();
        properties.setApiKey("");
        OpenAiCompatiblePostModerationClient client = new OpenAiCompatiblePostModerationClient(properties, new ObjectMapper());

        IllegalStateException exception = assertThrows(IllegalStateException.class, () -> client.moderate(sampleRequest()));

        assertEquals("POST_MODERATION_API_KEY is required when OpenAI-compatible moderation is enabled", exception.getMessage());
    }

    private HttpServer startServer(AtomicReference<String> authorization, AtomicReference<String> body, String responseBody) throws IOException {
        HttpServer httpServer = HttpServer.create(new InetSocketAddress("127.0.0.1", 0), 0);
        httpServer.createContext("/v1/chat/completions", exchange -> handle(exchange, authorization, body, responseBody));
        httpServer.start();
        return httpServer;
    }

    private void handle(HttpExchange exchange, AtomicReference<String> authorization, AtomicReference<String> body, String responseBody) throws IOException {
        authorization.set(exchange.getRequestHeaders().getFirst("Authorization"));
        body.set(new String(exchange.getRequestBody().readAllBytes(), StandardCharsets.UTF_8));
        byte[] bytes = responseBody.getBytes(StandardCharsets.UTF_8);
        exchange.getResponseHeaders().add("Content-Type", "application/json");
        exchange.sendResponseHeaders(200, bytes.length);
        exchange.getResponseBody().write(bytes);
        exchange.close();
    }

    private PostModerationRequest sampleRequest() {
        return new PostModerationRequest(
                UUID.randomUUID(),
                UUID.randomUUID(),
                "STUDY",
                "Study Partner",
                "Looking for a study buddy",
                "TEXT_PREFERENCE",
                "Weekends",
                null,
                null,
                "Library",
                2,
                "GPA above 3.0",
                null,
                List.of("study"),
                Map.of("studyGoal", "Pass final exam"));
    }
}
