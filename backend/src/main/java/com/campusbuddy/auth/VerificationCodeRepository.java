package com.campusbuddy.auth;

import org.springframework.data.jpa.repository.JpaRepository;

public interface VerificationCodeRepository extends JpaRepository<CampusEmailVerificationCodeEntity, String> {
}
