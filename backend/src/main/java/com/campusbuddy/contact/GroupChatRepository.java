package com.campusbuddy.contact;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface GroupChatRepository extends JpaRepository<GroupChat, Long> {

    Page<GroupChat> findByStatusOrderByUpdatedAtDesc(String status, Pageable pageable);

    @Query("SELECT g FROM GroupChat g JOIN GroupChatMember m ON g.id = m.groupChatId " +
            "WHERE m.userId = :userId AND m.status = 'JOINED' AND g.status = 'ACTIVE' " +
            "ORDER BY g.updatedAt DESC")
    Page<GroupChat> findByMemberUserIdOrderByUpdatedAtDesc(@Param("userId") UUID userId, Pageable pageable);

    @Query("SELECT g FROM GroupChat g JOIN GroupChatMember m ON g.id = m.groupChatId " +
            "WHERE m.userId = :userId AND m.status = 'JOINED'")
    List<GroupChat> findByMemberUserId(@Param("userId") UUID userId);

    @Query("SELECT g FROM GroupChat g WHERE g.creatorId = :creatorId AND g.status = 'ACTIVE'")
    Page<GroupChat> findByCreatorId(@Param("creatorId") UUID creatorId, Pageable pageable);

    Optional<GroupChat> findByIdAndStatus(Long id, String status);

    @Query("SELECT g FROM GroupChat g WHERE g.relatedPostUuid = :postUuid AND g.status = 'ACTIVE'")
    Optional<GroupChat> findByRelatedPostUuid(@Param("postUuid") UUID postUuid);
}