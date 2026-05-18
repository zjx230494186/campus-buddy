package com.campusbuddy.auth;

public interface CampusEmailVerificationCodeSender {

    void send(String campusEmail, String verificationCode, String purpose);
}
