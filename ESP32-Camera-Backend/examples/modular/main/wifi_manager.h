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
    typedef bool (*AbortCallback)();

    bool connect(AbortCallback shouldAbort = nullptr);
    void disconnect();
    bool isConnected();
    IPAddress getIP();
    bool wasAborted() const;

private:
    unsigned long _timeout;
    bool _aborted;
};

#endif // WIFI_MANAGER_H
