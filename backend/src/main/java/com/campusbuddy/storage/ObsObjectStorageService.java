package com.campusbuddy.storage;

import com.campusbuddy.config.CampusBuddyProperties;
import com.obs.services.ObsClient;
import com.obs.services.model.DeleteObjectRequest;
import com.obs.services.model.GetObjectRequest;
import com.obs.services.model.ObjectMetadata;
import com.obs.services.model.ObsObject;
import com.obs.services.model.PutObjectRequest;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

public class ObsObjectStorageService implements ObjectStorageService {

    private final ObsClient obsClient;
    private final String bucketName;

    public ObsObjectStorageService(CampusBuddyProperties.ObjectStorage config) {
        String endpoint = config.getEndpoint();
        String accessKeyId = config.getAccessKeyId();
        String secretAccessKey = config.getSecretAccessKey();
        if (accessKeyId == null || accessKeyId.isBlank()) {
            throw new IllegalArgumentException(
                "OBS accessKeyId is required for deploy profile. "
                + "Set campus-buddy.object-storage.access-key-id via environment variable.");
        }
        if (secretAccessKey == null || secretAccessKey.isBlank()) {
            throw new IllegalArgumentException(
                "OBS secretAccessKey is required for deploy profile. "
                + "Set campus-buddy.object-storage.secret-access-key via environment variable.");
        }
        this.obsClient = new ObsClient(accessKeyId, secretAccessKey, endpoint);
        this.bucketName = config.getBucket();
    }

    @Override
    public void putObject(String key, String contentType, byte[] data) {
        ObjectMetadata metadata = new ObjectMetadata();
        metadata.setContentType(contentType);
        metadata.setContentLength((long) data.length);
        PutObjectRequest request = new PutObjectRequest();
        request.setBucketName(bucketName);
        request.setObjectKey(key);
        request.setMetadata(metadata);
        request.setInput(new ByteArrayInputStream(data));
        obsClient.putObject(request);
    }

    @Override
    public InputStream getObject(String key) {
        GetObjectRequest request = new GetObjectRequest();
        request.setBucketName(bucketName);
        request.setObjectKey(key);
        ObsObject obsObject = obsClient.getObject(request);
        if (obsObject == null) {
            return null;
        }
        return obsObject.getObjectContent();
    }

    @Override
    public void deleteObject(String key) {
        DeleteObjectRequest request = new DeleteObjectRequest();
        request.setBucketName(bucketName);
        request.setObjectKey(key);
        obsClient.deleteObject(request);
    }

    public void close() throws IOException {
        if (obsClient != null) {
            obsClient.close();
        }
    }
}
