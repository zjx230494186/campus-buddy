package com.campusbuddy.auth;

import com.campusbuddy.config.CampusBuddyProperties;
import jakarta.mail.internet.MimeMessage;
import org.junit.jupiter.api.Test;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.MimeMessagePreparator;

import java.io.InputStream;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatThrownBy;

class SmtpCampusEmailVerificationCodeSenderTest {

    @Test
    void sendBuildsVerificationEmailWithCodeAndPurpose() {
        CapturingJavaMailSender mailSender = new CapturingJavaMailSender();
        CampusBuddyProperties properties = smtpProperties();
        SmtpCampusEmailVerificationCodeSender sender = new SmtpCampusEmailVerificationCodeSender(mailSender, properties);

        sender.send("student@campus.edu.cn", "123456", "REGISTER_OR_LOGIN");

        assertThat(mailSender.message).isNotNull();
        assertThat(mailSender.message.getTo()).containsExactly("student@campus.edu.cn");
        assertThat(mailSender.message.getFrom()).isEqualTo("校园搭子平台 <noreply@campus.edu.cn>");
        assertThat(mailSender.message.getSubject()).isEqualTo("校园搭子平台验证码");
        assertThat(mailSender.message.getText()).contains("123456");
        assertThat(mailSender.message.getText()).contains("注册或登录校园账号");
        assertThat(mailSender.message.getText()).contains("10 分钟");
    }

    @Test
    void sendFailsFastWhenSmtpHostMissing() {
        CapturingJavaMailSender mailSender = new CapturingJavaMailSender();
        CampusBuddyProperties properties = smtpProperties();
        properties.getCampusEmail().getSmtp().setHost("");
        SmtpCampusEmailVerificationCodeSender sender = new SmtpCampusEmailVerificationCodeSender(mailSender, properties);

        assertThatThrownBy(() -> sender.send("student@campus.edu.cn", "123456", "REGISTER_OR_LOGIN"))
                .isInstanceOf(IllegalStateException.class)
                .hasMessageContaining("SMTP host");
    }

    private CampusBuddyProperties smtpProperties() {
        CampusBuddyProperties properties = new CampusBuddyProperties();
        properties.getCampusEmail().setCodeExpiresInSeconds(600);
        properties.getCampusEmail().setDeliveryMode("smtp");
        CampusBuddyProperties.CampusEmail.Smtp smtp = properties.getCampusEmail().getSmtp();
        smtp.setHost("smtp.example.test");
        smtp.setPort(587);
        smtp.setUsername("noreply@campus.edu.cn");
        smtp.setPassword("test-password");
        smtp.setFrom("noreply@campus.edu.cn");
        smtp.setFromName("校园搭子平台");
        return properties;
    }

    static class CapturingJavaMailSender implements JavaMailSender {
        private SimpleMailMessage message;

        @Override
        public MimeMessage createMimeMessage() {
            throw new UnsupportedOperationException();
        }

        @Override
        public MimeMessage createMimeMessage(InputStream contentStream) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void send(MimeMessage mimeMessage) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void send(MimeMessage... mimeMessages) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void send(MimeMessagePreparator mimeMessagePreparator) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void send(MimeMessagePreparator... mimeMessagePreparators) {
            throw new UnsupportedOperationException();
        }

        @Override
        public void send(SimpleMailMessage simpleMessage) {
            this.message = simpleMessage;
        }

        @Override
        public void send(SimpleMailMessage... simpleMessages) {
            this.message = simpleMessages[0];
        }
    }
}
