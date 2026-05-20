package com.campusbuddy.review;

import org.springframework.data.jpa.repository.JpaRepository;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface ReviewRepository extends JpaRepository<Review, Long> {
    boolean existsByConversationIdAndReviewerIdAndRevieweeId(Long conversationId, UUID reviewerId, UUID revieweeId);
    List<Review> findByRevieweeIdAndStatus(UUID revieweeId, String status);
    List<Review> findByReviewerId(UUID reviewerId);
    Optional<Review> findByIdAndReviewerId(Long id, UUID reviewerId);
}
