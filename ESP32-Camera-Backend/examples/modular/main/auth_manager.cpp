/**
 * auth_manager.cpp - Authentication management implementation
 */

#include <Arduino.h>
#include "auth_manager.h"
#include "server_resolver.h"

// Static RTC memory variable
RTC_DATA_ATTR char AuthManager::_rtcToken[512] = "";

AuthManager::AuthManager() {
    _token = "";
}

bool AuthManager::login() {
    Serial.println("Logging in to server...");
    
    HTTPClient http;
    String loginUrl = serverResolver.buildApiUrl("/auth/login");
    http.begin(loginUrl);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(10000);
    
    // Create JSON payload
    DynamicJsonDocument loginDoc(256);
    loginDoc["username"] = USERNAME;
    loginDoc["password"] = USER_PASSWORD;
    
    String requestBody;
    serializeJson(loginDoc, requestBody);
    
    Serial.println("POST " + loginUrl);
    
    // Send request
    int httpCode = http.POST(requestBody);
    bool success = false;
    
    if (httpCode > 0) {
        String response = http.getString();
        Serial.printf("HTTP %d\n", httpCode);
        
        if (httpCode == 200) {
            DynamicJsonDocument responseDoc(2048);
            DeserializationError error = deserializeJson(responseDoc, response);
            
            if (!error && responseDoc["success"] && responseDoc["data"]["token"]) {
                _token = responseDoc["data"]["token"].as<String>();
                Serial.println("✓ Token received");
                saveTokenToRTC(_token.c_str());
                success = true;
            } else {
                Serial.println("JSON parse error or no token");
            }
        }
    } else {
        Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
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
