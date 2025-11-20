/**
 * mqtt_manager.cpp - MQTT Manager Implementation
 */

#include <Arduino.h>
#include "mqtt_manager.h"
#include "config.h"

MQTTManager::MQTTManager(const char* brokerAddr, int brokerPort, const char* id)
    : broker(brokerAddr), port(brokerPort), clientId(id), mqttClient(wifiClient) {
    topicImage = MQTT_TOPIC_IMAGE;
    topicStatus = MQTT_TOPIC_STATUS;
    
    // Set buffer size for images
    // Note: PubSubClient has limits on message size
    // Default: 256 bytes, Max practical: ~128KB
    mqttClient.setBufferSize(131072); // 128KB buffer
    mqttClient.setSocketTimeout(30);  // 30 second timeout for large messages
}

bool MQTTManager::connect() {
    Serial.println("🔌 Connecting to MQTT broker...");
    Serial.printf("Broker: %s:%d\n", broker, port);
    
    mqttClient.setServer(broker, port);
    mqttClient.setKeepAlive(60);
    
    // Try to connect with retry
    int retries = 0;
    while (!mqttClient.connected() && retries < 5) {
        Serial.printf("Attempt %d/5...\n", retries + 1);
        
        if (mqttClient.connect(clientId)) {
            Serial.println("✅ MQTT connected!");
            publishStatus("online");
            return true;
        }
        
        Serial.printf("❌ Failed, rc=%d. Retry in 2s...\n", mqttClient.state());
        delay(2000);
        retries++;
    }
    
    Serial.println("❌ MQTT connection failed!");
    return false;
}

bool MQTTManager::publishImage(const uint8_t* imageData, size_t imageSize) {
    // Check if image is too large for MQTT
    const size_t MAX_MQTT_SIZE = 100000; // 100KB limit for stability
    
    if (imageSize > MAX_MQTT_SIZE) {
        Serial.printf("⚠️ Image too large for MQTT (%d bytes > %d bytes)\n", imageSize, MAX_MQTT_SIZE);
        Serial.println("💡 Use HTTP upload for large images");
        return false; // Force HTTP fallback
    }
    
    if (!mqttClient.connected()) {
        Serial.println("⚠️ MQTT not connected, attempting reconnect...");
        if (!connect()) {
            return false;
        }
    }
    
    Serial.printf("📤 Publishing image (%d bytes) to MQTT...\n", imageSize);
    
    // Publish image data (non-retained for memory)
    bool success = mqttClient.publish(topicImage, imageData, imageSize, false);
    
    if (success) {
        Serial.println("✅ Image published to MQTT!");
    } else {
        Serial.printf("❌ Failed to publish! (Error: %d)\n", mqttClient.state());
        Serial.println("💡 Try reducing image quality in config.h");
    }
    
    return success;
}

bool MQTTManager::publishStatus(const char* status) {
    if (!mqttClient.connected()) {
        return false;
    }
    
    Serial.printf("📊 Publishing status: %s\n", status);
    return mqttClient.publish(topicStatus, status);
}

bool MQTTManager::isConnected() {
    return mqttClient.connected();
}

void MQTTManager::disconnect() {
    if (mqttClient.connected()) {
        publishStatus("offline");
        mqttClient.disconnect();
        Serial.println("🔌 MQTT disconnected");
    }
}
