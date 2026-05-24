package com.campusbuddy.contact;

import com.campusbuddy.common.ApiException;
import org.springframework.http.HttpStatus;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.time.Instant;
import java.util.UUID;

@Service
public class ContactCardService {

    private final ContactCardRepository contactCardRepository;

    public ContactCardService(ContactCardRepository contactCardRepository) {
        this.contactCardRepository = contactCardRepository;
    }

    @Transactional(readOnly = true)
    public ContactCardResponse getContactCard(UUID userId) {
        return contactCardRepository.findByUserId(userId)
                .map(card -> new ContactCardResponse(
                        true, card.getWechatId(), card.getPhoneNumber(), card.getQqNumber(), card.getUpdatedAt()))
                .orElseGet(() -> new ContactCardResponse(false, null, null, null, null));
    }

    @Transactional
    public ContactCardResponse upsertContactCard(UUID userId, String wechatId, String phoneNumber, String qqNumber) {
        validateAtLeastOneContact(wechatId, phoneNumber, qqNumber);
        validatePhoneNumber(phoneNumber);
        validateLength(wechatId, 64, "wechatId");
        validateLength(qqNumber, 32, "qqNumber");

        ContactCard card = contactCardRepository.findByUserId(userId).orElse(null);
        if (card == null) {
            card = new ContactCard(userId, wechatId, phoneNumber, qqNumber);
        } else {
            card.setWechatId(wechatId);
            card.setPhoneNumber(phoneNumber);
            card.setQqNumber(qqNumber);
            card.setUpdatedAt(Instant.now());
        }
        contactCardRepository.save(card);
        return new ContactCardResponse(true, card.getWechatId(), card.getPhoneNumber(), card.getQqNumber(), card.getUpdatedAt());
    }

    private void validateAtLeastOneContact(String wechatId, String phoneNumber, String qqNumber) {
        boolean hasAny = (wechatId != null && !wechatId.isBlank())
                || (phoneNumber != null && !phoneNumber.isBlank())
                || (qqNumber != null && !qqNumber.isBlank());
        if (!hasAny) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "At least one contact method is required", null);
        }
    }

    private void validatePhoneNumber(String phoneNumber) {
        if (phoneNumber != null && !phoneNumber.isBlank()) {
            if (!phoneNumber.matches("\\d{11}")) {
                throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", "Phone number must be exactly 11 digits", null);
            }
        }
    }

    private void validateLength(String value, int maxLen, String fieldName) {
        if (value != null && value.length() > maxLen) {
            throw new ApiException(HttpStatus.BAD_REQUEST, "VALIDATION_FAILED", fieldName + " exceeds maximum length of " + maxLen, null);
        }
    }

    public record ContactCardResponse(boolean hasCard, String wechatId, String phoneNumber, String qqNumber, Instant updatedAt) {}
}
