package com.campusbuddy.post;

import com.campusbuddy.TestcontainersConfiguration;
import com.campusbuddy.auth.CampusEmailVerificationCodeSender;
import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.context.TestConfiguration;
import org.springframework.boot.webmvc.test.autoconfigure.AutoConfigureMockMvc;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Import;
import org.springframework.context.annotation.Primary;
import org.springframework.http.MediaType;
import org.springframework.test.web.servlet.MockMvc;

import java.time.Clock;
import java.time.Instant;
import java.time.ZoneOffset;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import static org.hamcrest.Matchers.*;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.*;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.*;

@SpringBootTest
@AutoConfigureMockMvc
@Import(TestcontainersConfiguration.class)
class PartnerPostPlazaEndpointTest {

    @Autowired private MockMvc mockMvc;
    @Autowired private CapturingCampusEmailVerificationCodeSender codeSender;
    @Autowired private UserAccountRepository userAccountRepository;
    @Autowired private PartnerPostRepository partnerPostRepository;

    @Test
    void plazaListRequiresAuthentication() throws Exception {
        mockMvc.perform(get("/api/partner-posts"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void plazaDetailRequiresAuthentication() throws Exception {
        mockMvc.perform(get("/api/partner-posts/00000000-0000-0000-0000-000000000000"))
                .andExpect(status().isUnauthorized());
    }

    @Test
    void unverifiedUserCanViewPlazaList() throws Exception {
        String token = registerAndLogin("pl-unver@campus.edu.cn", "Str0ngPassword!", "PLUnver");
        mockMvc.perform(get("/api/partner-posts")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk());
    }

    @Test
    void plazaListOnlyReturnsPublished() throws Exception {
        String token = registerVerifiedAndLogin("pl-pub@campus.edu.cn", "Str0ngPassword!", "PLPub");
        java.util.UUID userId = getUserId("pl-pub@campus.edu.cn");

        PartnerPost draftPost = new PartnerPost(userId, "DRAFT", Instant.now());
        draftPost.setTitle("Draft Post");
        partnerPostRepository.save(draftPost);

        PartnerPost publishedPost = new PartnerPost(userId, "PUBLISHED", Instant.now());
        publishedPost.setTitle("Published Post");
        publishedPost.setSceneType("STUDY");
        publishedPost.setPublishedAt(Instant.now());
        partnerPostRepository.save(publishedPost);

        mockMvc.perform(get("/api/partner-posts")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))))
                .andExpect(jsonPath("$.items[?(@.title=='Draft Post')]").doesNotExist())
                .andExpect(jsonPath("$.items[?(@.title=='Published Post')]").exists());
    }

    @Test
    void plazaListSortedByPublishedAtDesc() throws Exception {
        String token = registerVerifiedAndLogin("pl-sort@campus.edu.cn", "Str0ngPassword!", "PLSort");
        java.util.UUID userId = getUserId("pl-sort@campus.edu.cn");

        PartnerPost older = new PartnerPost(userId, "PUBLISHED", Instant.now());
        older.setTitle("Older Post");
        older.setSceneType("SPORT");
        older.setPublishedAt(Instant.parse("2026-05-20T00:00:00Z"));
        partnerPostRepository.save(older);

        PartnerPost newer = new PartnerPost(userId, "PUBLISHED", Instant.now());
        newer.setTitle("Newer Post");
        newer.setSceneType("SPORT");
        newer.setPublishedAt(Instant.parse("2026-05-22T00:00:00Z"));
        partnerPostRepository.save(newer);

        mockMvc.perform(get("/api/partner-posts?sceneType=SPORT&size=2")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items[0].title").value("Newer Post"))
                .andExpect(jsonPath("$.items[1].title").value("Older Post"));
    }

    @Test
    void plazaListFilterBySceneType() throws Exception {
        String token = registerVerifiedAndLogin("pl-scene@campus.edu.cn", "Str0ngPassword!", "PLScene");
        java.util.UUID userId = getUserId("pl-scene@campus.edu.cn");

        PartnerPost studyPost = new PartnerPost(userId, "PUBLISHED", Instant.now());
        studyPost.setTitle("Study Post");
        studyPost.setSceneType("STUDY");
        studyPost.setPublishedAt(Instant.now());
        partnerPostRepository.save(studyPost);

        PartnerPost mealPost = new PartnerPost(userId, "PUBLISHED", Instant.now());
        mealPost.setTitle("Meal Post");
        mealPost.setSceneType("MEAL");
        mealPost.setPublishedAt(Instant.now().plusSeconds(1));
        partnerPostRepository.save(mealPost);

        mockMvc.perform(get("/api/partner-posts?sceneType=STUDY")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items[?(@.sceneType=='MEAL')]").doesNotExist())
                .andExpect(jsonPath("$.items[?(@.sceneType=='STUDY')]").exists());
    }

    @Test
    void plazaListKeywordSearch() throws Exception {
        String token = registerVerifiedAndLogin("pl-kw@campus.edu.cn", "Str0ngPassword!", "PLKw");
        java.util.UUID userId = getUserId("pl-kw@campus.edu.cn");

        PartnerPost post = new PartnerPost(userId, "PUBLISHED", Instant.now());
        post.setTitle("Calculus Study Group");
        post.setDescription("Preparing for final exam");
        post.setLocationText("South Library");
        post.setSceneType("STUDY");
        post.setTags(java.util.List.of("math", "finals"));
        post.setPublishedAt(Instant.now());
        partnerPostRepository.save(post);

        mockMvc.perform(get("/api/partner-posts?keyword=calculus")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))));

        mockMvc.perform(get("/api/partner-posts?keyword=SOUTH LIBRARY")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))));

        mockMvc.perform(get("/api/partner-posts?keyword=math")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.items", hasSize(greaterThanOrEqualTo(1))));

        mockMvc.perform(get("/api/partner-posts?keyword=zzznonexistent")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.totalElements").value(0));
    }

    @Test
    void plazaDetailOnlyAllowsPublished() throws Exception {
        String token = registerVerifiedAndLogin("pl-detpub@campus.edu.cn", "Str0ngPassword!", "PLDetPub");
        java.util.UUID userId = getUserId("pl-detpub@campus.edu.cn");

        PartnerPost draftPost = new PartnerPost(userId, "DRAFT", Instant.now());
        draftPost.setTitle("Secret Draft");
        partnerPostRepository.save(draftPost);

        mockMvc.perform(get("/api/partner-posts/" + draftPost.getId())
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isNotFound());
    }

    @Test
    void plazaDetailNonPublishedReturnsPostNotFound() throws Exception {
        String token = registerVerifiedAndLogin("pl-detnf@campus.edu.cn", "Str0ngPassword!", "PLDetNF");
        java.util.UUID userId = getUserId("pl-detnf@campus.edu.cn");

        PartnerPost rejectedPost = new PartnerPost(userId, "REJECTED", Instant.now());
        rejectedPost.setTitle("Rejected Post");
        partnerPostRepository.save(rejectedPost);

        mockMvc.perform(get("/api/partner-posts/" + rejectedPost.getId())
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isNotFound());
    }

    @Test
    void plazaListAndDetailExcludeSensitiveFields() throws Exception {
        String token = registerVerifiedAndLogin("pl-nosens@campus.edu.cn", "Str0ngPassword!", "PLNosens");
        java.util.UUID userId = getUserId("pl-nosens@campus.edu.cn");

        PartnerPost post = new PartnerPost(userId, "PUBLISHED", Instant.now());
        post.setTitle("Sensitivity Test");
        post.setSceneType("STUDY");
        post.setContactPreference("my wechat id");
        post.setPublishedAt(Instant.now());
        partnerPostRepository.save(post);

        String listResponse = mockMvc.perform(get("/api/partner-posts")
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        String detailResponse = mockMvc.perform(get("/api/partner-posts/" + post.getId())
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        for (String field : new String[]{"campusEmail", "studentNumber", "realName", "objectKey", "contactPreference", "rejectReason", "reviewedBy"}) {
            org.junit.jupiter.api.Assertions.assertFalse(
                    listResponse.contains("\"" + field + "\""),
                    "List must not contain: " + field
            );
            org.junit.jupiter.api.Assertions.assertFalse(
                    detailResponse.contains("\"" + field + "\""),
                    "Detail must not contain: " + field
            );
        }
    }

    @Test
    void publicCreditSummaryExcludesDisputedCount() throws Exception {
        String token = registerVerifiedAndLogin("pl-credit@campus.edu.cn", "Str0ngPassword!", "PLCredit");
        java.util.UUID userId = getUserId("pl-credit@campus.edu.cn");

        PartnerPost post = new PartnerPost(userId, "PUBLISHED", Instant.now());
        post.setTitle("Credit Test");
        post.setSceneType("STUDY");
        post.setPublishedAt(Instant.now());
        partnerPostRepository.save(post);

        String response = mockMvc.perform(get("/api/partner-posts/" + post.getId())
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();

        org.junit.jupiter.api.Assertions.assertFalse(
                response.contains("disputedReviewCount"),
                "Detail must not contain disputedReviewCount"
        );
    }

    @Test
    void ownPostTrueForOwnPublishedPost() throws Exception {
        String token = registerVerifiedAndLogin("pl-own@campus.edu.cn", "Str0ngPassword!", "PLOwn");
        java.util.UUID userId = getUserId("pl-own@campus.edu.cn");

        PartnerPost post = new PartnerPost(userId, "PUBLISHED", Instant.now());
        post.setTitle("Own Post Test");
        post.setSceneType("STUDY");
        post.setPublishedAt(Instant.now());
        partnerPostRepository.save(post);

        mockMvc.perform(get("/api/partner-posts/" + post.getId())
                        .header("Authorization", "Bearer " + token))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.ownPost").value(true));

        String otherToken = registerVerifiedAndLogin("pl-other@campus.edu.cn", "Str0ngPassword!", "PLOther");
        mockMvc.perform(get("/api/partner-posts/" + post.getId())
                        .header("Authorization", "Bearer " + otherToken))
                .andExpect(status().isOk())
                .andExpect(jsonPath("$.ownPost").value(false));
    }

    private java.util.UUID getUserId(String email) {
        return userAccountRepository.findByCampusEmail(email).map(UserAccount::getUserId).orElseThrow();
    }

    private String registerVerifiedAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        UserAccount account = userAccountRepository.findByCampusEmail(email).orElseThrow();
        account.setAuthenticationStatus("VERIFIED");
        userAccountRepository.save(account);
        return login(email, password);
    }

    private String registerAndLogin(String email, String password, String displayName) throws Exception {
        String ticket = verifiedTicket(email);
        mockMvc.perform(post("/api/auth/register")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"verificationTicket\":\"%s\",\"password\":\"%s\",\"displayName\":\"%s\"}".formatted(email, ticket, password, displayName)))
                .andExpect(status().isOk());
        return login(email, password);
    }

    private String login(String email, String password) throws Exception {
        String response = mockMvc.perform(post("/api/auth/login")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"password\":\"%s\"}".formatted(email, password)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.accessToken");
    }

    private String verifiedTicket(String email) throws Exception {
        mockMvc.perform(post("/api/auth/campus-email/verification-codes")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"purpose\":\"REGISTER_OR_LOGIN\"}".formatted(email)))
                .andExpect(status().isOk());
        String code = codeSender.latestCode(email, "REGISTER_OR_LOGIN");
        String response = mockMvc.perform(post("/api/auth/campus-email/verifications")
                        .contentType(MediaType.APPLICATION_JSON)
                        .content("{\"campusEmail\":\"%s\",\"code\":\"%s\",\"purpose\":\"REGISTER_OR_LOGIN\"}".formatted(email, code)))
                .andExpect(status().isOk())
                .andReturn().getResponse().getContentAsString();
        return com.jayway.jsonpath.JsonPath.read(response, "$.verificationTicket");
    }

    @TestConfiguration
    static class TestConfig {
        @Bean @Primary CapturingCampusEmailVerificationCodeSender capturingSender() { return new CapturingCampusEmailVerificationCodeSender(); }
        @Bean @Primary Clock testClock() { return Clock.fixed(Instant.parse("2026-05-23T00:00:00Z"), ZoneOffset.UTC); }
    }

    static class CapturingCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {
        private final Map<String, String> codes = new ConcurrentHashMap<>();
        @Override public void send(String campusEmail, String verificationCode, String purpose) { codes.put(campusEmail.toLowerCase() + "|" + purpose.toUpperCase(), verificationCode); }
        String latestCode(String campusEmail, String purpose) { return codes.get(campusEmail.toLowerCase() + "|" + purpose.toUpperCase()); }
    }
}
