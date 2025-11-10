/**
 * mqtt_manager.cpp - MQTT Manager Implementation
 */

#include <Arduino.h>
#include "mqtt_manager.h"
#include "config.h"

MQTTManager::MQTTManager(const char* brokerAddr, int brokerPort, const char* id)
    : brokerHost(brokerAddr), port(brokerPort), clientId(id), mqttClient(wifiClient) {
    topicImage = MQTT_TOPIC_IMAGE;
    topicStatus = MQTT_TOPIC_STATUS;

    mqttClient.setBufferSize(131072); // 128KB buffer
    mqttClient.setSocketTimeout(30);  // 30 second timeout for large messages
}

void MQTTManager::updateBroker(const String& host) {
    if (host.length() == 0) {
        return;
    }
    brokerHost = host;
    Serial.print("[MQTT] Broker target set to ");
    Serial.print(brokerHost);
    Serial.print(":");
    Serial.println(port);
}

bool MQTTManager::connect() {
    Serial.println("[MQTT] Connecting to broker...");
    Serial.printf("Broker: %s:%d\n", brokerHost.c_str(), port);

    mqttClient.setServer(brokerHost.c_str(), port);
    mqttClient.setKeepAlive(60);

    int retries = 0;
    while (!mqttClient.connected() && retries < 5) {
        Serial.printf("Attempt %d/5...\n", retries + 1);

        if (mqttClient.connect(clientId)) {
            Serial.println("[MQTT] Connected");
            publishStatus("online");
            return true;
        }

        Serial.printf("[MQTT] Failed, rc=%d. Retry in 2s...\n", mqttClient.state());
        delay(2000);
        retries++;
    }

    Serial.println("[MQTT] Connection failed");
    return false;
}

bool MQTTManager::publishImage(const uint8_t* imageData, size_t imageSize) {
    const size_t MAX_MQTT_SIZE = 100000; // 100KB limit for stability

    if (imageSize > MAX_MQTT_SIZE) {
        Serial.printf("[MQTT] Image too large (%d bytes > %d bytes)\n", imageSize, MAX_MQTT_SIZE);
        Serial.println("[MQTT] Use HTTP upload for large images");
        return false;
    }

    if (!mqttClient.connected()) {
        Serial.println("[MQTT] Not connected, attempting reconnect...");
        if (!connect()) {
            return false;
        }
    }

    Serial.printf("[MQTT] Publishing image (%d bytes)\n", imageSize);

    bool success = mqttClient.publish(topicImage, imageData, imageSize, false);

    if (success) {
        Serial.println("[MQTT] Image published");
    } else {
        Serial.printf("[MQTT] Publish failed (Error: %d)\n", mqttClient.state());
    }

    return success;
}

bool MQTTManager::publishStatus(const char* status) {
    if (!mqttClient.connected()) {
        return false;
    }

    Serial.printf("[MQTT] Publishing status: %s\n", status);
    return mqttClient.publish(topicStatus, status);
}

bool MQTTManager::isConnected() {
    return mqttClient.connected();
}

void MQTTManager::disconnect() {
    if (mqttClient.connected()) {
        publishStatus("offline");
        mqttClient.disconnect();
        Serial.println("[MQTT] Disconnected");
    }
}
