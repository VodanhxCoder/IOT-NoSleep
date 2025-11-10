/**
 * mqtt_manager.h - MQTT Manager Header
 * Handles MQTT connection and publishing
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

class MQTTManager {
private:
    WiFiClient wifiClient;
    PubSubClient mqttClient;
    String brokerHost;
    int port;
    const char* clientId;
    const char* topicImage;
    const char* topicStatus;

public:
    MQTTManager(const char* brokerAddr, int brokerPort, const char* id);
    void updateBroker(const String& host);
    bool connect();
    bool publishImage(const uint8_t* imageData, size_t imageSize);
    bool publishStatus(const char* status);
    bool isConnected();
    void disconnect();
};

#endif
