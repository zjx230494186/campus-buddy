package com.campusbuddy.contact;

import com.campusbuddy.common.ApiException;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.List;
import java.util.UUID;

@Service
public class ContactUnlockService {

    private final ConversationRepository conversationRepository;
    private final ContactUnlockConfirmRepository contactUnlockConfirmRepository;
    private final ContactUnlockRecordRepository contactUnlockRecordRepository;
    private final ContactCardRepository contactCardRepository;

    public ContactUnlockService(ConversationRepository conversationRepository,
                                ContactUnlockConfirmRepository contactUnlockConfirmRepository,
                                ContactUnlockRecordRepository contactUnlockRecordRepository,
                                ContactCardRepository contactCardRepository) {
        this.conversationRepository = conversationRepository;
        this.contactUnlockConfirmRepository = contactUnlockConfirmRepository;
        this.contactUnlockRecordRepository = contactUnlockRecordRepository;
        this.contactCardRepository = contactCardRepository;
    }

    @Transactional(readOnly = true)
    public ContactUnlockStatusResponse getUnlockStatus(UUID currentUserId, Long conversationId) {
        Conversation conv = findConversationOrThrow(conversationId);
        validateParticipant(conv, currentUserId);

        UUID peerId = conv.getOtherParticipant(currentUserId);
        boolean currentUserConfirmed = contactUnlockConfirmRepository.existsByConversationIdAndUserId(conversationId, currentUserId);
        boolean peerConfirmed = contactUnlockConfirmRepository.existsByConversationIdAndUserId(conversationId, peerId);
        boolean currentUserHasCard = contactCardRepository.existsByUserId(currentUserId);
        boolean peerHasCard = contactCardRepository.existsByUserId(peerId);
        boolean alreadyUnlocked = contactUnlockRecordRepository.existsByConversationId(conversationId);

        String status;
        Instant unlockedAt = null;
        if (alreadyUnlocked) {
            status = "UNLOCKED";
            var unlockRecord = contactUnlockRecordRepository.findByConversationId(conversationId).orElse(null);
            if (unlockRecord != null) {
                unlockedAt = unlockRecord.getUnlockedAt();
            }
        } else if (currentUserConfirmed) {
            status = "WAITING_FOR_PEER";
        } else {
            status = "LOCKED";
        }

        return new ContactUnlockStatusResponse(conversationId, status, currentUserConfirmed, peerConfirmed,
                currentUserHasCard, peerHasCard, unlockedAt);
    }

    @Transactional
    public ContactUnlockStatusResponse confirmUnlock(UUID currentUserId, Long conversationId) {
        Conversation conv = findConversationOrThrow(conversationId);
        validateParticipant(conv, currentUserId);

        if (!"ACTIVE".equals(conv.getStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "CONVERSATION_CLOSED", "Cannot confirm unlock on a non-ACTIVE conversation", null);
        }

        if (!contactCardRepository.existsByUserId(currentUserId)) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "CONTACT_CARD_REQUIRED", "Current user must have a contact card before confirming", null);
        }

        if (!contactUnlockConfirmRepository.existsByConversationIdAndUserId(conversationId, currentUserId)) {
            contactUnlockConfirmRepository.save(new ContactUnlockConfirm(conversationId, currentUserId));
        }

        UUID peerId = conv.getOtherParticipant(currentUserId);
        boolean peerConfirmed = contactUnlockConfirmRepository.existsByConversationIdAndUserId(conversationId, peerId);
        boolean peerHasCard = contactCardRepository.existsByUserId(peerId);

        if (peerConfirmed && peerHasCard && !contactUnlockRecordRepository.existsByConversationId(conversationId)) {
            Instant now = Instant.now();
            ContactUnlockRecord record = new ContactUnlockRecord(conversationId, now);
            contactUnlockRecordRepository.save(record);
        }

        return getUnlockStatus(currentUserId, conversationId);
    }

    @Transactional(readOnly = true)
    public PeerContactCardResponse getPeerContactCard(UUID currentUserId, Long conversationId) {
        Conversation conv = findConversationOrThrow(conversationId);
        validateParticipant(conv, currentUserId);

        if (!"ACTIVE".equals(conv.getStatus())) {
            throw new ApiException(HttpStatus.FORBIDDEN, "CONTACT_UNLOCK_NOT_AVAILABLE", "Cannot view peer contact card on a non-ACTIVE conversation", null);
        }

        if (!contactUnlockRecordRepository.existsByConversationId(conversationId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "CONTACT_NOT_UNLOCKED", "Contact information has not been unlocked", null);
        }

        UUID peerId = conv.getOtherParticipant(currentUserId);
        ContactCard peerCard = contactCardRepository.findByUserId(peerId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "PEER_CONTACT_CARD_NOT_FOUND", "Peer does not have a contact card", null));

        return new PeerContactCardResponse(peerCard.getWechatId(), peerCard.getPhoneNumber(), peerCard.getQqNumber());
    }

    private Conversation findConversationOrThrow(Long conversationId) {
        return conversationRepository.findById(conversationId)
                .orElseThrow(() -> new ApiException(HttpStatus.NOT_FOUND, "CONVERSATION_NOT_FOUND", "Conversation not found", null));
    }

    private void validateParticipant(Conversation conv, UUID userId) {
        if (!conv.isParticipant(userId)) {
            throw new ApiException(HttpStatus.FORBIDDEN, "NOT_PARTICIPANT", "User is not a participant of this conversation", null);
        }
    }

    public record ContactUnlockStatusResponse(Long conversationId, String status, boolean currentUserConfirmed,
                                               boolean peerConfirmed, boolean currentUserHasContactCard,
                                               boolean peerHasContactCard, Instant unlockedAt) {}

    public record PeerContactCardResponse(String wechatId, String phoneNumber, String qqNumber) {}
}
