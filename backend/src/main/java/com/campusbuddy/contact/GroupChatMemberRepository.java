package com.campusbuddy.contact;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.data.jpa.repository.Query;
import org.springframework.data.repository.query.Param;

import java.util.List;
import java.util.Optional;
import java.util.UUID;

public interface GroupChatMemberRepository extends JpaRepository<GroupChatMember, Long> {

    Optional<GroupChatMember> findByGroupChatIdAndUserId(Long groupChatId, UUID userId);

    List<GroupChatMember> findByGroupChatIdAndStatus(Long groupChatId, String status);

    Page<GroupChatMember> findByGroupChatIdAndStatus(Long groupChatId, String status, Pageable pageable);

    long countByGroupChatIdAndStatus(Long groupChatId, String status);

    @Query("SELECT m FROM GroupChatMember m WHERE m.groupChatId = :groupChatId AND m.status = 'JOINED' AND m.role = 'ADMIN'")
    List<GroupChatMember> findAdminsByGroupChatId(@Param("groupChatId") Long groupChatId);

    @Query("SELECT m FROM GroupChatMember m WHERE m.userId = :userId AND m.status = 'JOINED'")
    List<GroupChatMember> findByUserIdAndStatus(@Param("userId") UUID userId);

    @Query("SELECT m FROM GroupChatMember m WHERE m.groupChatId = :groupChatId AND m.userId = :userId AND m.status = 'JOINED'")
    Optional<GroupChatMember> findActiveMember(@Param("groupChatId") Long groupChatId, @Param("userId") UUID userId);
}