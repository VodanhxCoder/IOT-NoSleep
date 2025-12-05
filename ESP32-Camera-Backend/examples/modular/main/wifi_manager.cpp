/**
 * wifi_manager.cpp - WiFi management implementation
 */

#include <Arduino.h>
#include "wifi_manager.h"

WiFiManager::WiFiManager() {
    _timeout = WIFI_TIMEOUT_MS;
    _aborted = false;
}

bool WiFiManager::connect(AbortCallback shouldAbort) {
    _aborted = false;
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (shouldAbort && shouldAbort()) {
            Serial.println("\n[WiFi] Aborted by callback");
            _aborted = true;
            return false;
        }

        if (millis() - startTime > _timeout) {
            Serial.println("\n�o- WiFi timeout!");
            return false;
        }

        delay(500);
        Serial.print(".");
    }

    Serial.println("\n�o\" WiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    return true;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    Serial.println("WiFi disconnected");
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiManager::getIP() {
    return WiFi.localIP();
}

bool WiFiManager::wasAborted() const {
    return _aborted;
}
