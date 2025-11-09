/**
 * upload_manager.cpp - Upload management implementation
 */

#include <Arduino.h>
#include <ESPmDNS.h>
#include "upload_manager.h"

// Helper function to resolve hostname to IP
String resolveHostnameForUpload(const char* hostname) {
    // Check if it's already an IP address
    if (strchr(hostname, '.') && !strstr(hostname, ".local")) {
        return String(hostname);
    }
    
    // Try to resolve mDNS
    IPAddress serverIP = MDNS.queryHost(hostname);
    
    if (serverIP.toString() == "0.0.0.0") {
        // Fallback to SERVER_IP
        return String(SERVER_IP);
    }
    
    return serverIP.toString();
}

UploadManager::UploadManager() {
    _lastHttpCode = 0;
    _lastResponse = "";
}

bool UploadManager::upload(camera_fb_t* fb, const String& token) {
    if (!fb) {
        Serial.println("✗ Invalid frame buffer");
        return false;
    }
    
    // Resolve hostname
    String resolvedIP = resolveHostnameForUpload(SERVER_HOSTNAME);
    String uploadUrl = "http://" + resolvedIP + ":3000/api/upload-image";
    
    HTTPClient http;
    http.begin(uploadUrl);
    http.addHeader("Authorization", "Bearer " + token);
    http.addHeader("Content-Type", "image/jpeg");
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

bool UploadManager::uploadImage(const uint8_t* buf, size_t len, const String& token) {
    if (!buf || len == 0) {
        Serial.println("✗ Invalid buffer");
        return false;
    }
    
    // Resolve hostname
    String resolvedIP = resolveHostnameForUpload(SERVER_HOSTNAME);
    String uploadUrl = "http://" + resolvedIP + ":3000/api/upload-image";
    
    HTTPClient http;
    http.begin(uploadUrl);
    http.addHeader("Authorization", "Bearer " + token);
    http.setTimeout(30000); // 30s timeout
    
    Serial.println("📤 Uploading image to server...");
    Serial.printf("📡 Upload URL: %s\n", uploadUrl.c_str());
    Serial.printf("📏 Image size: %d bytes\n", len);
    
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
    Serial.println("⏳ Sending HTTP POST...");
    _lastHttpCode = http.POST(fullBody, totalLen);
    free(fullBody);
    
    bool success = false;
    Serial.printf("📊 HTTP Response Code: %d\n", _lastHttpCode);
    
    if (_lastHttpCode > 0) {
        _lastResponse = http.getString();
        Serial.printf("✅ HTTP %d\n", _lastHttpCode);
        Serial.printf("📄 Response: %s\n", _lastResponse.substring(0, 200).c_str());
        
        if (_lastHttpCode == 200 || _lastHttpCode == 201) {
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, _lastResponse);
            
            if (!error && doc["success"]) {
                const char* message = doc["message"] | "Success";
                Serial.println(message);
                success = true;
            }
        } else if (_lastHttpCode == 401) {
            Serial.println("❌ Token expired (401)");
        } else {
            Serial.printf("❌ Error response: %s\n", _lastResponse.substring(0, 200).c_str());
        }
    } else {
        Serial.printf("❌ HTTP Error: %s (code: %d)\n", http.errorToString(_lastHttpCode).c_str(), _lastHttpCode);
        Serial.println("🔍 Troubleshooting:");
        Serial.println("   - Check backend is running on 192.168.77.24:3000");
        Serial.println("   - Check upload endpoint: /api/upload-image");
        Serial.println("   - Try: curl http://192.168.77.24:3000/health");
    }
    
    http.end();
    return success;
}
