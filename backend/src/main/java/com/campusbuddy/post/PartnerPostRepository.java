package com.campusbuddy.post;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.Optional;
import java.util.UUID;

public interface PartnerPostRepository extends JpaRepository<PartnerPost, UUID> {
    Page<PartnerPost> findByPublisherId(UUID publisherId, Pageable pageable);
    Page<PartnerPost> findByPublisherIdAndStatus(UUID publisherId, String status, Pageable pageable);
    Optional<PartnerPost> findByIdAndPublisherId(UUID id, UUID publisherId);
    Page<PartnerPost> findByStatus(String status, Pageable pageable);
    long countByPublisherIdAndStatus(UUID publisherId, String status);

    @Query("SELECT p FROM PartnerPost p WHERE p.status = 'PUBLISHED' AND p.sceneType = :sceneType ORDER BY p.publishedAt DESC")
    Page<PartnerPost> findPublishedBySceneType(@Param("sceneType") String sceneType, Pageable pageable);

    @Query("SELECT p FROM PartnerPost p WHERE p.status = 'PUBLISHED' AND " +
            "(LOWER(p.title) LIKE LOWER(CONCAT('%', :keyword, '%')) OR " +
            "LOWER(p.description) LIKE LOWER(CONCAT('%', :keyword, '%')) OR " +
            "LOWER(p.locationText) LIKE LOWER(CONCAT('%', :keyword, '%')) OR " +
            "LOWER(CAST(p.tags AS string)) LIKE LOWER(CONCAT('%', :keyword, '%'))) ORDER BY p.publishedAt DESC")
    Page<PartnerPost> searchPublishedByKeyword(@Param("keyword") String keyword, Pageable pageable);

    @Query("SELECT p FROM PartnerPost p WHERE p.status = 'PUBLISHED' AND p.sceneType = :sceneType AND " +
            "(LOWER(p.title) LIKE LOWER(CONCAT('%', :keyword, '%')) OR " +
            "LOWER(p.description) LIKE LOWER(CONCAT('%', :keyword, '%')) OR " +
            "LOWER(p.locationText) LIKE LOWER(CONCAT('%', :keyword, '%')) OR " +
            "LOWER(CAST(p.tags AS string)) LIKE LOWER(CONCAT('%', :keyword, '%'))) ORDER BY p.publishedAt DESC")
    Page<PartnerPost> searchPublishedBySceneTypeAndKeyword(@Param("sceneType") String sceneType, @Param("keyword") String keyword, Pageable pageable);

    @Query("SELECT p FROM PartnerPost p WHERE p.status = 'PUBLISHED' ORDER BY p.publishedAt DESC")
    Page<PartnerPost> findAllPublished(Pageable pageable);

    Optional<PartnerPost> findByIdAndStatus(UUID id, String status);
}
