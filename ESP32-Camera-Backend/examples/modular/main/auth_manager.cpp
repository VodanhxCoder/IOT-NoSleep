/**
 * auth_manager.cpp - Authentication management implementation
 */

#include <Arduino.h>
#include <ESPmDNS.h>
#include "auth_manager.h"

// Static RTC memory variable
RTC_DATA_ATTR char AuthManager::_rtcToken[512] = "";

AuthManager::AuthManager() {
    _token = "";
}

// Helper function to resolve mDNS hostname to IP
String resolveHostname(const char* hostname) {
    Serial.printf("🔍 Resolving hostname: %s\n", hostname);
    
    // Check if it's already an IP address
    if (strchr(hostname, '.') && !strstr(hostname, ".local")) {
        Serial.printf("✅ Already IP address: %s\n", hostname);
        return String(hostname);
    }
    
    // Try to resolve mDNS
    IPAddress serverIP = MDNS.queryHost(hostname);
    
    if (serverIP.toString() == "0.0.0.0") {
        Serial.printf("❌ mDNS resolve failed for: %s\n", hostname);
        Serial.printf("🔄 Falling back to: %s\n", SERVER_IP);
        return String(SERVER_IP);
    }
    
    Serial.printf("✅ Resolved to: %s\n", serverIP.toString().c_str());
    return serverIP.toString();
}

bool AuthManager::login() {
    Serial.println("Logging in to server...");
    
    // Resolve hostname if using mDNS
    String serverHost = SERVER_HOSTNAME;
    String resolvedIP = resolveHostname(serverHost.c_str());
    
    HTTPClient http;
    String loginUrl = "http://" + resolvedIP + ":3000/api/auth/login";
    Serial.printf("📡 Connecting to: %s\n", loginUrl.c_str());
    
    http.begin(loginUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    // Create JSON payload
    DynamicJsonDocument loginDoc(256);
    loginDoc["username"] = USERNAME;
    loginDoc["password"] = USER_PASSWORD;
    
    String requestBody;
    serializeJson(loginDoc, requestBody);
    
    Serial.println("📤 POST " + loginUrl);
    Serial.printf("📦 Payload: %s\n", requestBody.c_str());
    
    // Send request
    Serial.println("⏳ Sending HTTP request...");
    int httpCode = http.POST(requestBody);
    bool success = false;
    
    Serial.printf("📊 HTTP Response Code: %d\n", httpCode);
    
    if (httpCode > 0) {
        String response = http.getString();
        Serial.printf("✅ HTTP %d\n", httpCode);
        Serial.printf("📄 Response: %s\n", response.substring(0, 200).c_str());
        
        if (httpCode == 200) {
            DynamicJsonDocument responseDoc(2048);
            DeserializationError error = deserializeJson(responseDoc, response);
            
            if (!error && responseDoc["success"] && responseDoc["data"]["token"]) {
                _token = responseDoc["data"]["token"].as<String>();
                Serial.println("✅ Token received");
                saveTokenToRTC(_token.c_str());
                success = true;
            } else {
                Serial.println("❌ JSON parse error or no token");
                if (error) {
                    Serial.printf("❌ JSON error: %s\n", error.c_str());
                }
            }
        } else {
            Serial.printf("❌ HTTP Error %d: %s\n", httpCode, response.substring(0, 100).c_str());
        }
    } else {
        Serial.printf("❌ HTTP Error: %s (code: %d)\n", http.errorToString(httpCode).c_str(), httpCode);
        Serial.println("🔍 Troubleshooting:");
        Serial.println("   - Check PC backend is running");
        Serial.println("   - Check firewall allows port 3000");
        Serial.printf("   - Backend IP: %s\n", resolvedIP.c_str());
    }
    
    http.end();
    return success;
}

String AuthManager::getToken() {
    return _token;
}

void AuthManager::clearToken() {
    _token = "";
    _rtcToken[0] = '\0';
}

bool AuthManager::hasToken() {
    return _token.length() > 0;
}

void AuthManager::saveTokenToRTC(const char* token) {
    strncpy(_rtcToken, token, sizeof(_rtcToken) - 1);
    _rtcToken[sizeof(_rtcToken) - 1] = '\0';
}

bool AuthManager::restoreTokenFromRTC() {
    if (strlen(_rtcToken) > 0) {
        _token = String(_rtcToken);
        Serial.println("✓ Token restored from RTC memory");
        return true;
    }
    return false;
}

bool AuthManager::ensureLoggedIn() {
    // Try to restore token from RTC memory first
    if (restoreTokenFromRTC()) {
        Serial.println("Using cached token from RTC");
        return true;
    }
    
    // No token, need to login
    Serial.println("No cached token, logging in...");
    return login();
}
