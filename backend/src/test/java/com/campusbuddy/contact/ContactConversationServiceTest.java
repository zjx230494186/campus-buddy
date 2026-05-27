package com.campusbuddy.contact;

import com.campusbuddy.auth.UserAccount;
import com.campusbuddy.auth.UserAccountRepository;
import com.campusbuddy.common.ApiException;
import com.campusbuddy.post.PartnerPost;
import com.campusbuddy.post.PartnerPostRepository;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.extension.ExtendWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.junit.jupiter.MockitoExtension;
import org.springframework.test.util.ReflectionTestUtils;

import java.time.Instant;
import java.util.Optional;
import java.util.UUID;

import static org.junit.jupiter.api.Assertions.*;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.Mockito.*;

@ExtendWith(MockitoExtension.class)
class ContactConversationServiceTest {

    @Mock private ConversationRepository conversationRepository;
    @Mock private ConversationMessageRepository conversationMessageRepository;
    @Mock private PartnerPostRepository partnerPostRepository;
    @Mock private UserAccountRepository userAccountRepository;

    @Test
    void requesterCannotSendSecondMessageBeforePublisherReplies() {
        UUID requesterId = UUID.randomUUID();
        UUID publisherId = UUID.randomUUID();
        Conversation conversation = activeConversation(10L, requesterId, publisherId);

        when(conversationRepository.findById(10L)).thenReturn(Optional.of(conversation));
        when(conversationMessageRepository.countByConversationIdAndSenderIdAndMessageType(10L, requesterId, "USER_TEXT"))
                .thenReturn(1L);
        when(conversationMessageRepository.countUserTextFromOther(10L, requesterId)).thenReturn(0L);

        ContactConversationService service = service();

        ApiException ex = assertThrows(ApiException.class,
                () -> service.sendMessage(requesterId, 10L, "follow up"));

        assertEquals("CONTACT_REPLY_REQUIRED", ex.code());
        verify(conversationMessageRepository, never()).save(any());
    }

    @Test
    void requesterCanSendAfterPublisherReplies() {
        UUID requesterId = UUID.randomUUID();
        UUID publisherId = UUID.randomUUID();
        Conversation conversation = activeConversation(10L, requesterId, publisherId);

        when(conversationRepository.findById(10L)).thenReturn(Optional.of(conversation));
        when(conversationMessageRepository.countByConversationIdAndSenderIdAndMessageType(10L, requesterId, "USER_TEXT"))
                .thenReturn(1L);
        when(conversationMessageRepository.countUserTextFromOther(10L, requesterId)).thenReturn(1L);
        when(conversationMessageRepository.save(any(ConversationMessage.class))).thenAnswer(invocation -> {
            ConversationMessage msg = invocation.getArgument(0);
            ReflectionTestUtils.setField(msg, "id", 99L);
            return msg;
        });

        ContactConversationService service = service();

        ContactConversationService.SendMessageResponse response = service.sendMessage(requesterId, 10L, "follow up");

        assertEquals(99L, response.messageId());
        verify(conversationRepository).save(conversation);
    }

    @Test
    void repeatedContactRequestBeforePublisherRepliesIsRejected() {
        UUID requesterId = UUID.randomUUID();
        UUID publisherId = UUID.randomUUID();
        UUID postId = UUID.randomUUID();
        PartnerPost post = publishedPost(postId, publisherId);
        Conversation conversation = activeConversation(10L, requesterId, publisherId);

        when(partnerPostRepository.findByIdAndStatus(postId, "PUBLISHED")).thenReturn(Optional.of(post));
        when(userAccountRepository.findById(requesterId)).thenReturn(Optional.of(verifiedUser()));
        when(userAccountRepository.findById(publisherId)).thenReturn(Optional.of(verifiedUser()));
        when(conversationRepository.findActiveByParticipants(requesterId, publisherId)).thenReturn(Optional.of(conversation));
        when(conversationMessageRepository.countByConversationIdAndSenderIdAndMessageType(10L, requesterId, "USER_TEXT"))
                .thenReturn(1L);
        when(conversationMessageRepository.countUserTextFromOther(10L, requesterId)).thenReturn(0L);

        ContactConversationService service = service();

        ApiException ex = assertThrows(ApiException.class,
                () -> service.requestContact(requesterId, postId, "second invite"));

        assertEquals("CONTACT_REPLY_REQUIRED", ex.code());
        verify(conversationMessageRepository, never()).save(any());
    }

    private ContactConversationService service() {
        return new ContactConversationService(
                conversationRepository,
                conversationMessageRepository,
                partnerPostRepository,
                userAccountRepository
        );
    }

    private static Conversation activeConversation(Long id, UUID participant1, UUID participant2) {
        Conversation conversation = new Conversation(participant1, participant2, "ACTIVE", Instant.now());
        ReflectionTestUtils.setField(conversation, "id", id);
        return conversation;
    }

    private static PartnerPost publishedPost(UUID postId, UUID publisherId) {
        PartnerPost post = new PartnerPost(publisherId, "PUBLISHED", Instant.now());
        ReflectionTestUtils.setField(post, "id", postId);
        post.setPublishedAt(Instant.now());
        return post;
    }

    private static UserAccount verifiedUser() {
        UserAccount account = new UserAccount("user-" + UUID.randomUUID() + "@campus.edu.cn", "hash", "User", Instant.now());
        account.setAuthenticationStatus("VERIFIED");
        return account;
    }
}
