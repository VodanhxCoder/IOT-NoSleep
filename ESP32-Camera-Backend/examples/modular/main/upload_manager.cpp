/**
 * upload_manager.cpp - Upload management implementation
 */

#include <Arduino.h>
#include "upload_manager.h"
#include "server_resolver.h"

UploadManager::UploadManager() {
    _lastHttpCode = 0;
    _lastResponse = "";
}

bool UploadManager::upload(camera_fb_t* fb, const String& token) {
    if (!fb) {
        Serial.println("✗ Invalid frame buffer");
        return false;
    }
    
    HTTPClient http;
    String uploadUrl = serverResolver.buildApiUrl("/upload-image");
    http.begin(uploadUrl);
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "image/jpeg");
    http.addHeader("X-Image-Encrypted", "0");
    http.setTimeout(30000); // 30s timeout
    
    Serial.println("📤 Uploading to server...");
    _lastHttpCode = http.POST(fb->buf, fb->len);
    
    bool success = false;
    if (_lastHttpCode > 0) {
        _lastResponse = http.getString();
        Serial.printf("HTTP %d\n", _lastHttpCode);
        
        if (_lastHttpCode == 200 || _lastHttpCode == 201) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, _lastResponse);
            
            if (!error && doc["success"]) {
                const char* message = doc["message"] | "Success";
                Serial.println(message);
                success = true;
            }
        } else if (_lastHttpCode == 401) {
            Serial.println("Token expired (401)");
        }
    } else {
        Serial.printf("HTTP Error: %s\n", http.errorToString(_lastHttpCode).c_str());
    }
    
    http.end();
    return success;
}

int UploadManager::getLastHttpCode() {
    return _lastHttpCode;
}

String UploadManager::getLastResponse() {
    return _lastResponse;
}

bool UploadManager::uploadImage(const uint8_t* buf, size_t len, const String& token, const String& ivBase64) {
    if (!buf || len == 0) {
        Serial.println("✗ Invalid buffer");
        return false;
    }
    
    HTTPClient http;
    String uploadUrl = serverResolver.buildApiUrl("/upload-image");
    http.begin(uploadUrl);
    http.addHeader("Authorization", "Bearer " + token);
    if (ivBase64.length() > 0) {
        http.addHeader("X-Image-Encrypted", "1");
        http.addHeader("X-Image-IV", ivBase64);
    } else {
        http.addHeader("X-Image-Encrypted", "0");
    }
    http.setTimeout(30000); // 30s timeout
    
    Serial.println("📤 Uploading image to server...");
    
    // Create multipart/form-data boundary
    String boundary = "----ESP32Boundary" + String(millis());
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);
    
    // Build multipart body
    String bodyStart = "--" + boundary + "\r\n";
    bodyStart += "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n";
    bodyStart += "Content-Type: image/jpeg\r\n\r\n";
    
    String bodyEnd = "\r\n--" + boundary + "--\r\n";
    
    // Calculate total size
    size_t totalLen = bodyStart.length() + len + bodyEnd.length();
    
    // Allocate buffer for complete body
    uint8_t* fullBody = (uint8_t*)malloc(totalLen);
    if (!fullBody) {
        Serial.println("✗ Memory allocation failed");
        http.end();
        return false;
    }
    
    // Copy parts into buffer
    memcpy(fullBody, bodyStart.c_str(), bodyStart.length());
    memcpy(fullBody + bodyStart.length(), buf, len);
    memcpy(fullBody + bodyStart.length() + len, bodyEnd.c_str(), bodyEnd.length());
    
    // Send POST request
    _lastHttpCode = http.POST(fullBody, totalLen);
    free(fullBody);
    
    bool success = false;
    if (_lastHttpCode > 0) {
        _lastResponse = http.getString();
        Serial.printf("HTTP %d\n", _lastHttpCode);
        
        if (_lastHttpCode == 200 || _lastHttpCode == 201) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, _lastResponse);
            
            if (!error && doc["success"]) {
                const char* message = doc["message"] | "Success";
                Serial.println(message);
                success = true;
            }
        } else if (_lastHttpCode == 401) {
            Serial.println("Token expired (401)");
        } else {
            Serial.println("Response: " + _lastResponse.substring(0, 200));
        }
    } else {
        Serial.printf("HTTP Error: %s\n", http.errorToString(_lastHttpCode).c_str());
    }
    
    http.end();
    return success;
}
