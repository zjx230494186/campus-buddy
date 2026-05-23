package com.campusbuddy.security;

import com.campusbuddy.common.ApiErrorResponse;
import com.campusbuddy.common.TraceIdFilter;
import com.fasterxml.jackson.databind.ObjectMapper;
import jakarta.servlet.http.HttpServletRequest;
import jakarta.servlet.http.HttpServletResponse;
import org.springframework.boot.context.properties.EnableConfigurationProperties;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.http.MediaType;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configurers.AbstractHttpConfigurer;
import org.springframework.security.config.http.SessionCreationPolicy;
import org.springframework.security.web.AuthenticationEntryPoint;
import org.springframework.security.web.SecurityFilterChain;
import org.springframework.security.web.access.AccessDeniedHandler;
import org.springframework.security.web.authentication.UsernamePasswordAuthenticationFilter;

@Configuration
@EnableConfigurationProperties({JwtPlaceholderProperties.class, JwtProperties.class})
public class SecurityConfiguration {

    @Bean
    SecurityFilterChain securityFilterChain(
            HttpSecurity http,
            JwtAuthenticationFilter jwtAuthenticationFilter,
            AuthenticationEntryPoint authenticationEntryPoint,
            AccessDeniedHandler accessDeniedHandler
    ) throws Exception {
        return http
                .csrf(AbstractHttpConfigurer::disable)
                .sessionManagement(session -> session.sessionCreationPolicy(SessionCreationPolicy.STATELESS))
                .exceptionHandling(exceptions -> exceptions
                        .authenticationEntryPoint(authenticationEntryPoint)
                        .accessDeniedHandler(accessDeniedHandler))
                .authorizeHttpRequests(authorize -> authorize
                        .requestMatchers("/api/admin/**").hasRole("ADMIN")
                        .requestMatchers("/api/probe/secure").authenticated()
                        .requestMatchers("/api/auth/identity-verifications", "/api/auth/identity-verifications/me", "/api/auth/identity-verifications/materials", "/api/auth/identity-verifications/materials/*").authenticated()
                        .requestMatchers("/api/me/reviews/**").authenticated()
                        .requestMatchers("/api/partner-posts/**").authenticated()
                        .requestMatchers("/api/me/partner-posts/**").authenticated()
                        .requestMatchers("/api/me/credit-summary").authenticated()
                        .requestMatchers("/api/users/*/credit-summary").authenticated()
                        .requestMatchers("/api/health", "/api/system/info").permitAll()
                        .anyRequest().permitAll()
                )
                .addFilterBefore(jwtAuthenticationFilter, UsernamePasswordAuthenticationFilter.class)
                .build();
    }

    @Bean
    AuthenticationEntryPoint authenticationEntryPoint() {
        ObjectMapper objectMapper = new ObjectMapper();
        return (request, response, authException) -> {
            response.setStatus(HttpServletResponse.SC_UNAUTHORIZED);
            response.setContentType(MediaType.APPLICATION_JSON_VALUE);
            ApiErrorResponse errorResponse = new ApiErrorResponse(
                    "UNAUTHORIZED",
                    "Authentication required",
                    "Missing or invalid bearer token",
                    traceId(request)
            );
            objectMapper.writeValue(response.getOutputStream(), errorResponse);
        };
    }

    @Bean
    AccessDeniedHandler accessDeniedHandler() {
        ObjectMapper objectMapper = new ObjectMapper();
        return (request, response, accessDeniedException) -> {
            response.setStatus(HttpServletResponse.SC_FORBIDDEN);
            response.setContentType(MediaType.APPLICATION_JSON_VALUE);
            ApiErrorResponse errorResponse = new ApiErrorResponse(
                    "FORBIDDEN",
                    "Access denied",
                    "Insufficient permissions",
                    traceId(request)
            );
            objectMapper.writeValue(response.getOutputStream(), errorResponse);
        };
    }

    private static String traceId(HttpServletRequest request) {
        Object traceId = request.getAttribute(TraceIdFilter.TRACE_ID_ATTRIBUTE);
        if (traceId instanceof String value && !value.isBlank()) {
            return value;
        }
        return "missing-trace-id";
    }
}
