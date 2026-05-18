package com.campusbuddy.storage;

import java.io.InputStream;

public interface ObjectStorageService {

    void putObject(String key, String contentType, byte[] data);

    InputStream getObject(String key);
}
