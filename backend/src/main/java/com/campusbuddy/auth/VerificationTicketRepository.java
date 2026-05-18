package com.campusbuddy.auth;

import org.springframework.data.jpa.repository.JpaRepository;

public interface VerificationTicketRepository extends JpaRepository<CampusEmailVerificationTicketEntity, String> {
}
