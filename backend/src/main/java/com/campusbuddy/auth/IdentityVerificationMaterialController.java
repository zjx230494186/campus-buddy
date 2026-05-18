package com.campusbuddy.auth;

import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.security.core.Authentication;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestPart;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import java.io.IOException;
import java.util.UUID;

@RestController
@RequestMapping("/api/auth/identity-verifications")
public class IdentityVerificationMaterialController {

    private final IdentityMaterialAttachmentService attachmentService;

    IdentityVerificationMaterialController(IdentityMaterialAttachmentService attachmentService) {
        this.attachmentService = attachmentService;
    }

    @PostMapping(value = "/materials", consumes = MediaType.MULTIPART_FORM_DATA_VALUE, produces = MediaType.APPLICATION_JSON_VALUE)
    ResponseEntity<IdentityMaterialAttachmentService.UploadResponse> uploadMaterial(
            Authentication authentication,
            @RequestPart("file") MultipartFile file
    ) throws IOException {
        UUID userId = UUID.fromString(authentication.getName());
        IdentityMaterialAttachmentService.UploadResponse response = attachmentService.upload(
                userId, file.getContentType(), file.getOriginalFilename(), file.getBytes()
        );
        return ResponseEntity.ok(response);
    }
}
