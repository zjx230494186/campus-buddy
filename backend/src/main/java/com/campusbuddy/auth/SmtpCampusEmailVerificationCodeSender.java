package com.campusbuddy.auth;

import com.campusbuddy.config.CampusBuddyProperties;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.mail.MailException;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.stereotype.Component;
import org.springframework.util.StringUtils;

@Component
@ConditionalOnProperty(prefix = "campus-buddy.campus-email", name = "delivery-mode", havingValue = "smtp")
class SmtpCampusEmailVerificationCodeSender implements CampusEmailVerificationCodeSender {

    private final JavaMailSender mailSender;
    private final CampusBuddyProperties campusBuddyProperties;

    SmtpCampusEmailVerificationCodeSender(JavaMailSender mailSender, CampusBuddyProperties campusBuddyProperties) {
        this.mailSender = mailSender;
        this.campusBuddyProperties = campusBuddyProperties;
    }

    @Override
    public void send(String campusEmail, String verificationCode, String purpose) {
        CampusBuddyProperties.CampusEmail.Smtp smtp = campusBuddyProperties.getCampusEmail().getSmtp();
        validateSmtpConfig(smtp);

        SimpleMailMessage message = new SimpleMailMessage();
        message.setFrom(formatFrom(smtp));
        message.setTo(campusEmail);
        message.setSubject("校园搭子平台验证码");
        message.setText("""
                你的校园搭子平台验证码是：%s

                该验证码用于：%s。
                验证码有效期为 %d 分钟，请勿转发给他人。

                如果这不是你本人操作，请忽略本邮件。
                """.formatted(
                verificationCode,
                purposeLabel(purpose),
                Math.max(1, campusBuddyProperties.getCampusEmail().getCodeExpiresInSeconds() / 60)
        ));

        try {
            mailSender.send(message);
        } catch (MailException exception) {
            throw new IllegalStateException("Failed to send campus email verification code", exception);
        }
    }

    private void validateSmtpConfig(CampusBuddyProperties.CampusEmail.Smtp smtp) {
        if (!StringUtils.hasText(smtp.getHost())) {
            throw new IllegalStateException("SMTP host must be configured when campus email delivery-mode=smtp");
        }
        if (!StringUtils.hasText(smtp.getFrom())) {
            throw new IllegalStateException("SMTP from address must be configured when campus email delivery-mode=smtp");
        }
        if (smtp.isAuth() && (!StringUtils.hasText(smtp.getUsername()) || !StringUtils.hasText(smtp.getPassword()))) {
            throw new IllegalStateException("SMTP username and password must be configured when SMTP auth is enabled");
        }
    }

    private String formatFrom(CampusBuddyProperties.CampusEmail.Smtp smtp) {
        if (!StringUtils.hasText(smtp.getFromName())) {
            return smtp.getFrom();
        }
        return "%s <%s>".formatted(smtp.getFromName(), smtp.getFrom());
    }

    private String purposeLabel(String purpose) {
        if ("REGISTER_OR_LOGIN".equalsIgnoreCase(purpose)) {
            return "注册或登录校园账号";
        }
        return purpose;
    }
}
