package com.campusbuddy.storage;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class InMemoryObjectStorageService implements ObjectStorageService {

    private final Map<String, byte[]> store = new ConcurrentHashMap<>();

    @Override
    public void putObject(String key, String contentType, byte[] data) {
        store.put(key, data);
    }

    @Override
    public InputStream getObject(String key) {
        byte[] data = store.get(key);
        if (data == null) {
            return null;
        }
        return new ByteArrayInputStream(data);
    }

    @Override
    public void deleteObject(String key) {
        store.remove(key);
    }
}
