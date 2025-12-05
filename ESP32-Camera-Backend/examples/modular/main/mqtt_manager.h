/**
 * mqtt_manager.h - MQTT Manager Header
 * Handles MQTT connection and publishing
 */

#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <PubSubClient.h>
#include <WiFiClientSecure.h>

class MQTTManager {
private:
    WiFiClientSecure wifiClient;
    PubSubClient mqttClient;
    const char* broker;
    int port;
    const char* clientId;
    const char* username; // New
    const char* password; // New
    const char* topicImage;
    const char* topicStatus;
    const char* topicCommand;

public:
    MQTTManager(const char* brokerAddr, int brokerPort, const char* id, const char* user = NULL, const char* pass = NULL);
    bool connect();
    void setCallback(MQTT_CALLBACK_SIGNATURE); // New callback setter
    bool subscribe(const char* topic); // New subscribe method
    bool publishImage(const uint8_t* imageData, size_t imageSize);
    bool publishImageChunked(const uint8_t* imageData, size_t imageSize); // New chunked method
    bool publishStatus(const char* status);
    bool isConnected();
    void disconnect();
    void loop(); // Add loop method
};

#endif
