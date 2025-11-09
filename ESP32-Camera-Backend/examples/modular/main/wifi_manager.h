/**
 * wifi_manager.h - WiFi connection management
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "config.h"

class WiFiManager {
public:
    WiFiManager();
    bool connect();
    void disconnect();
    bool isConnected();
    IPAddress getIP();

private:
    unsigned long _timeout;
};

#endif // WIFI_MANAGER_H
