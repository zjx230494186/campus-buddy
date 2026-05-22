package com.campusbuddy.post;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;

import java.util.Optional;
import java.util.UUID;

public interface PartnerPostRepository extends JpaRepository<PartnerPost, UUID> {
    Page<PartnerPost> findByPublisherId(UUID publisherId, Pageable pageable);
    Page<PartnerPost> findByPublisherIdAndStatus(UUID publisherId, String status, Pageable pageable);
    Optional<PartnerPost> findByIdAndPublisherId(UUID id, UUID publisherId);
    Page<PartnerPost> findByStatus(String status, Pageable pageable);
    long countByPublisherIdAndStatus(UUID publisherId, String status);
}
