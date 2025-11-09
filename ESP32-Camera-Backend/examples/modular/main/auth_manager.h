/**
 * auth_manager.h - Authentication and token management
 */

#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

class AuthManager {
public:
    AuthManager();
    bool login();
    bool ensureLoggedIn();  // Added: auto-login if no token
    String getToken();
    void clearToken();
    bool hasToken();
    
    // RTC memory management
    void saveTokenToRTC(const char* token);
    bool restoreTokenFromRTC();

private:
    String _token;
    static char _rtcToken[512];  // RTC memory storage
};

#endif // AUTH_MANAGER_H
