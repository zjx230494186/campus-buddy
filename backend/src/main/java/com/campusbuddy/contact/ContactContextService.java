package com.campusbuddy.contact;

import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.List;
import java.util.UUID;

@Service
public class ContactContextService {

    private static final long MIN_USER_MESSAGES_FOR_VALID = 2;

    private final ConversationRepository conversationRepository;
    private final ConversationMessageRepository conversationMessageRepository;
    private final ContactUnlockRecordRepository contactUnlockRecordRepository;

    public ContactContextService(ConversationRepository conversationRepository,
                          ConversationMessageRepository conversationMessageRepository,
                          ContactUnlockRecordRepository contactUnlockRecordRepository) {
        this.conversationRepository = conversationRepository;
        this.conversationMessageRepository = conversationMessageRepository;
        this.contactUnlockRecordRepository = contactUnlockRecordRepository;
    }

    @Transactional(readOnly = true)
    public boolean isParticipant(Long conversationId, UUID userId) {
        return conversationRepository.findById(conversationId)
                .map(conv -> conv.isParticipant(userId))
                .orElse(false);
    }

    @Transactional(readOnly = true)
    public boolean isOtherParticipant(Long conversationId, UUID reviewerId, UUID revieweeId) {
        if (reviewerId.equals(revieweeId)) return false;
        return conversationRepository.findById(conversationId)
                .filter(conv -> conv.isParticipant(reviewerId) && conv.isParticipant(revieweeId))
                .isPresent();
    }

    @Transactional(readOnly = true)
    public long countUserMessages(Long conversationId, UUID userId) {
        var convOpt = conversationRepository.findById(conversationId);
        if (convOpt.isEmpty()) return 0;
        Conversation conv = convOpt.get();
        if (!conv.isParticipant(userId)) return 0;
        return conversationMessageRepository.countByConversationIdAndSenderIdAndMessageType(
                conversationId, userId, "USER_TEXT");
    }

    @Transactional(readOnly = true)
    public boolean isValidConversation(Long conversationId) {
        var convOpt = conversationRepository.findById(conversationId);
        if (convOpt.isEmpty()) return false;
        Conversation conv = convOpt.get();
        long p1Count = countUserMessages(conversationId, conv.getParticipant1Id());
        long p2Count = countUserMessages(conversationId, conv.getParticipant2Id());
        return p1Count >= MIN_USER_MESSAGES_FOR_VALID && p2Count >= MIN_USER_MESSAGES_FOR_VALID;
    }

    @Transactional(readOnly = true)
    public List<Conversation> findConversationsByParticipant(UUID userId) {
        return conversationRepository.findByParticipant1IdOrParticipant2Id(userId, userId);
    }

    @Transactional(readOnly = true)
    public boolean isContactUnlocked(Long conversationId) {
        return contactUnlockRecordRepository.existsByConversationId(conversationId);
    }
}
