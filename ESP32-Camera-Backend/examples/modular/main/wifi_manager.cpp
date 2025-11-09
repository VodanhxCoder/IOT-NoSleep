/**
 * wifi_manager.cpp - WiFi management implementation
 */

#include <Arduino.h>
#include "wifi_manager.h"

WiFiManager::WiFiManager() {
    _timeout = WIFI_TIMEOUT_MS;
}

bool WiFiManager::connect() {
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    
    // Configure Static IP (DISABLED - using DHCP to match router subnet)
    // IPAddress local_IP(192, 168, 2, 100);      // ESP32 IP
    // IPAddress gateway(192, 168, 2, 1);         // Router IP
    // IPAddress subnet(255, 255, 255, 0);
    // IPAddress primaryDNS(8, 8, 8, 8);          // Google DNS
    // IPAddress secondaryDNS(8, 8, 4, 4);
    
    // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    //     Serial.println("✗ Static IP config failed!");
    // }
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > _timeout) {
            Serial.println("\n✗ WiFi timeout!");
            return false;
        }
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\n✓ WiFi connected");
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
