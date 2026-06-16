package com.campusbuddy.moderation;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.util.List;
import java.util.Map;

public class OpenAiCompatiblePostModerationClient implements PostModerationClient {

    private final PostModerationProperties properties;
    private final ObjectMapper objectMapper;
    private final HttpClient httpClient;

    public OpenAiCompatiblePostModerationClient(PostModerationProperties properties) {
        this(properties, new ObjectMapper());
    }

    public OpenAiCompatiblePostModerationClient(PostModerationProperties properties, ObjectMapper objectMapper) {
        this.properties = properties;
        this.objectMapper = objectMapper;
        this.httpClient = HttpClient.newBuilder()
                .connectTimeout(Duration.ofMillis(properties.getTimeoutMillis()))
                .build();
    }

    @Override
    public PostModerationDecision moderate(PostModerationRequest request) {
        if (properties.getApiKey() == null || properties.getApiKey().isBlank()) {
            throw new IllegalStateException("POST_MODERATION_API_KEY is required when OpenAI-compatible moderation is enabled");
        }

        try {
            String requestBody = objectMapper.writeValueAsString(buildProviderRequest(request));
            HttpRequest httpRequest = HttpRequest.newBuilder()
                    .uri(URI.create(properties.getBaseUrl()))
                    .timeout(Duration.ofMillis(properties.getTimeoutMillis()))
                    .header("Authorization", "Bearer " + properties.getApiKey())
                    .header("Content-Type", "application/json")
                    .POST(HttpRequest.BodyPublishers.ofString(requestBody))
                    .build();

            HttpResponse<String> response = httpClient.send(httpRequest, HttpResponse.BodyHandlers.ofString());
            if (response.statusCode() < 200 || response.statusCode() >= 300) {
                throw new IllegalStateException("post moderation provider returned non-2xx status: " + response.statusCode());
            }

            JsonNode root = objectMapper.readTree(response.body());
            String content = root.path("choices").path(0).path("message").path("content").asText();
            if (content == null || content.isBlank()) {
                throw new IllegalStateException("post moderation provider returned empty content");
            }

            JsonNode decision = objectMapper.readTree(content);
            return new PostModerationDecision(
                    decision.path("decision").asText("NEEDS_HUMAN"),
                    decision.path("confidence").asDouble(0.0),
                    textOrNull(decision, "reason"),
                    textOrNull(decision, "userVisibleReason"),
                    textOrNull(decision, "category"));
        } catch (IOException e) {
            throw new IllegalStateException("post moderation provider response could not be parsed", e);
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            throw new IllegalStateException("post moderation provider request was interrupted", e);
        }
    }

    private Map<String, Object> buildProviderRequest(PostModerationRequest request) {
        String userPrompt = """
                请审核下面的校园搭子帖子。只输出 JSON，不要输出解释文本。

                输出 schema:
                {
                  "decision": "APPROVE" | "REJECT" | "NEEDS_HUMAN",
                  "confidence": 0.0-1.0,
                  "reason": "给管理员看的简短原因",
                  "userVisibleReason": "如果 REJECT，给用户看的中文原因；否则为 null",
                  "category": "HARASSMENT|ILLEGAL|SAFETY|PRIVACY|SPAM|OTHER|null"
                }

                审核策略:
                - 正常校园学习、运动、吃饭、组队内容可以 APPROVE。
                - 涉及违法交易、骚扰、歧视、暴力、自伤、色情、诈骗、明显隐私泄露、广告引流，应 REJECT。
                - 信息不足、语义含糊或不确定时，返回 NEEDS_HUMAN。
                - 不要因为普通联系方式偏好字段缺失而拒绝；系统已在提交前拦截直接手机号。

                帖子:
                %s
                """.formatted(toJson(request));

        return Map.of(
                "model", properties.getModel(),
                "temperature", 0,
                "response_format", Map.of("type", "json_object"),
                "messages", List.of(
                        Map.of("role", "system", "content", "你是校园搭子平台的内容安全审核助手，必须保守、稳定、只返回合法 JSON。"),
                        Map.of("role", "user", "content", userPrompt)
                )
        );
    }

    private String toJson(PostModerationRequest request) {
        try {
            return objectMapper.writeValueAsString(request);
        } catch (IOException e) {
            throw new IllegalStateException("post moderation request could not be serialized", e);
        }
    }

    private String textOrNull(JsonNode node, String field) {
        JsonNode value = node.get(field);
        if (value == null || value.isNull()) {
            return null;
        }
        String text = value.asText();
        return text.isBlank() ? null : text;
    }
}
