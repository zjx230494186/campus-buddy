package com.campusbuddy.config;

import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.mail.javamail.JavaMailSenderImpl;

import java.util.Properties;

@Configuration
class MailSenderConfiguration {

    @Bean
    @ConditionalOnProperty(prefix = "campus-buddy.campus-email", name = "delivery-mode", havingValue = "smtp")
    JavaMailSender campusBuddyJavaMailSender(CampusBuddyProperties properties) {
        CampusBuddyProperties.CampusEmail.Smtp smtp = properties.getCampusEmail().getSmtp();
        JavaMailSenderImpl sender = new JavaMailSenderImpl();
        sender.setHost(smtp.getHost());
        sender.setPort(smtp.getPort());
        sender.setUsername(smtp.getUsername());
        sender.setPassword(smtp.getPassword());

        Properties javaMailProperties = sender.getJavaMailProperties();
        javaMailProperties.put("mail.smtp.auth", Boolean.toString(smtp.isAuth()));
        javaMailProperties.put("mail.smtp.starttls.enable", Boolean.toString(smtp.isStartTls()));
        javaMailProperties.put("mail.smtp.ssl.enable", Boolean.toString(smtp.isSsl()));
        javaMailProperties.put("mail.smtp.connectiontimeout", Integer.toString(smtp.getConnectionTimeoutMillis()));
        javaMailProperties.put("mail.smtp.timeout", Integer.toString(smtp.getTimeoutMillis()));
        javaMailProperties.put("mail.smtp.writetimeout", Integer.toString(smtp.getWriteTimeoutMillis()));
        return sender;
    }
}
