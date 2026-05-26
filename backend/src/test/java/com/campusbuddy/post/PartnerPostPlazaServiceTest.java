package com.campusbuddy.post;

import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import com.campusbuddy.review.CreditSummaryService;
import org.junit.jupiter.api.Test;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.test.util.ReflectionTestUtils;

import java.time.Instant;
import java.util.Optional;
import java.util.UUID;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

class PartnerPostPlazaServiceTest {

    private final PartnerPostRepository postRepository = mock(PartnerPostRepository.class);
    private final UserAccountRepository userAccountRepository = mock(UserAccountRepository.class);
    private final CreditSummaryService creditSummaryService = mock(CreditSummaryService.class);
    private final PartnerPostPlazaService service = new PartnerPostPlazaService(postRepository, userAccountRepository, creditSummaryService);

    @Test
    void unverifiedUserCannotListPublishedPosts() {
        UUID currentUserId = UUID.randomUUID();
        when(userAccountRepository.findById(currentUserId)).thenReturn(Optional.of(accountWithStatus("UNVERIFIED")));
        when(postRepository.findAllPublished(any(Pageable.class))).thenReturn(Page.empty());

        assertThatThrownBy(() -> service.listPosts(currentUserId, null, null, 0, 20))
                .isInstanceOf(ApiException.class)
                .satisfies(error -> assertThat(((ApiException) error).code()).isEqualTo("AUTHENTICATION_STATUS_REQUIRED"));
    }

    @Test
    void unverifiedUserCannotViewPublishedPostDetail() {
        UUID currentUserId = UUID.randomUUID();
        UUID publisherId = UUID.randomUUID();
        UUID postId = UUID.randomUUID();
        PartnerPost post = new PartnerPost(publisherId, "PUBLISHED", Instant.now());
        ReflectionTestUtils.setField(post, "id", postId);

        when(userAccountRepository.findById(currentUserId)).thenReturn(Optional.of(accountWithStatus("UNVERIFIED")));
        when(postRepository.findByIdAndStatus(postId, "PUBLISHED")).thenReturn(Optional.of(post));
        when(userAccountRepository.findById(publisherId)).thenReturn(Optional.of(accountWithStatus("VERIFIED")));

        assertThatThrownBy(() -> service.getPostDetail(currentUserId, postId))
                .isInstanceOf(ApiException.class)
                .satisfies(error -> assertThat(((ApiException) error).code()).isEqualTo("AUTHENTICATION_STATUS_REQUIRED"));
    }

    private UserAccount accountWithStatus(String authenticationStatus) {
        UserAccount account = new UserAccount("test-%s@campus.edu.cn".formatted(UUID.randomUUID()), "hash", "Test", Instant.now());
        account.setAuthenticationStatus(authenticationStatus);
        return account;
    }
}
