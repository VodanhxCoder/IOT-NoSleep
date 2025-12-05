/**
 * mqtt_manager.cpp - MQTT Manager Implementation
 */

#include <Arduino.h>
#include <WiFi.h> // Added for WiFi.localIP()
#include "mqtt_manager.h"
#include "config.h"
#include <base64.h> // Requires base64 library

MQTTManager::MQTTManager(const char* brokerAddr, int brokerPort, const char* id, const char* user, const char* pass)
    : broker(brokerAddr), port(brokerPort), clientId(id), username(user), password(pass), mqttClient(wifiClient) {
    topicImage = MQTT_TOPIC_IMAGE;
    topicStatus = MQTT_TOPIC_STATUS;
    topicCommand = MQTT_TOPIC_COMMAND;
    
    // Set buffer size for images
    // Note: PubSubClient has limits on message size
    // Reduced to 8KB to save heap (since we use chunking now)
    mqttClient.setBufferSize(8192); 
    mqttClient.setSocketTimeout(30);  // 30 second timeout for large messages
    
    // Allow insecure TLS (skip certificate validation) for HiveMQ Cloud
    wifiClient.setInsecure();
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
        
        bool connected = false;
        if (username && password) {
             connected = mqttClient.connect(clientId, username, password);
        } else {
             connected = mqttClient.connect(clientId);
        }

        if (connected) {
            Serial.println("✅ MQTT connected!");
            
            // Publish rich status with IP and Stream URL
            String ip = WiFi.localIP().toString();
            String streamUrl = "http://" + ip + ":" + String(STREAM_PORT) + "/stream";
            String payload = "{\"status\":\"online\",\"ip\":\"" + ip + "\",\"streamUrl\":\"" + streamUrl + "\"}";
            
            publishStatus(payload.c_str());
            mqttClient.subscribe(topicCommand); // Auto subscribe to command topic
            mqttClient.subscribe("camera/server-ip"); // Subscribe to Server IP discovery
            return true;
        }
        
        Serial.printf("❌ Failed, rc=%d. Retry in 2s...\n", mqttClient.state());
        delay(2000);
        retries++;
    }
    
    Serial.println("❌ MQTT connection failed!");
    return false;
}

void MQTTManager::setCallback(MQTT_CALLBACK_SIGNATURE) {
    mqttClient.setCallback(callback);
}

bool MQTTManager::subscribe(const char* topic) {
    return mqttClient.subscribe(topic);
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

bool MQTTManager::publishImageChunked(const uint8_t* imageData, size_t imageSize) {
    if (!mqttClient.connected()) {
        if (!connect()) return false;
    }

    // 1. Encode entire image to Base64
    // Note: This requires significant RAM. If image is too large, this might fail.
    // For 40KB image -> ~54KB Base64 string
    // String base64Image = base64::encode(imageData, imageSize);
    // int totalLen = base64Image.length();
    
    // 2. Calculate chunks
    const int CHUNK_SIZE = 3072; // Multiple of 3 for valid Base64 chunks
    int totalLen = (imageSize + 2) / 3 * 4; // Base64 length
    int totalChunks = (imageSize + CHUNK_SIZE - 1) / CHUNK_SIZE;
    String imageId = String(millis()); // Simple unique ID
    
    Serial.printf("📦 Chunking image: %d bytes -> %d Base64 chars (%d chunks)\n", 
                  imageSize, totalLen, totalChunks);

    // 3. Send chunks
    for (int i = 0; i < totalChunks; i++) {
        int start = i * CHUNK_SIZE;
        int len = (start + CHUNK_SIZE > imageSize) ? (imageSize - start) : CHUNK_SIZE;
        
        // Encode just this chunk
        String chunkData = base64::encode(imageData + start, len);

        // Create JSON payload manually to save memory
        String payload = "{";
        payload += "\"id\":\"" + imageId + "\",";
        payload += "\"index\":" + String(i) + ",";
        payload += "\"total\":" + String(totalChunks) + ",";
        payload += "\"data\":\"" + chunkData + "\",";
        payload += "\"userId\":\"" + String(USERNAME) + "\""; // From config.h
        payload += "}";

        // Publish
        if (!mqttClient.publish(topicImage, payload.c_str())) {
            Serial.printf("❌ Failed to send chunk %d/%d\n", i+1, totalChunks);
            return false;
        }
        
        Serial.printf("📤 Sent chunk %d/%d\n", i+1, totalChunks);
        delay(50); // Small delay to prevent network congestion
    }
    
    Serial.println("✅ All chunks sent successfully!");
    return true;
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

void MQTTManager::loop() {
    mqttClient.loop();
}

void MQTTManager::disconnect() {
    if (mqttClient.connected()) {
        publishStatus("offline");
        mqttClient.disconnect();
        Serial.println("🔌 MQTT disconnected");
    }
}
