/**
 * main.ino - Main ESP32 Deep Sleep Motion Camera
 * Modular version - reduces memory load
 *
 * Hardware:
 * - ESP32-S3-EYE
 * - PIR Sensor on GPIO14 (wake source)
 * - WS2812 LED on GPIO48
 *
 * Features:
 * - Motion-triggered wake from deep sleep
 * - Auto-login with JWT token (cached in RTC memory)
 * - Camera capture + upload to server
 * - Ultra low power: ~10uA in sleep (~6 months on 3000mAh)
 */

#include "config.h"
#include <ESPmDNS.h>
#include "wifi_manager.h"
#include "auth_manager.h"
#include "led_manager.h"
#include "camera_manager.h"
#include "upload_manager.h"
#include "mqtt_manager.h" // Include MQTT Manager
#include "stream_manager.h" // Include Stream Manager
#include "storage_manager.h" // Re-include Storage Manager

// Manager instances
WiFiManager wifiMgr;
AuthManager authMgr;
LEDManager ledMgr;
CameraManager cameraMgr;
UploadManager uploadMgr;
MQTTManager mqttMgr(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD); // Initialize MQTT Manager
StreamManager streamMgr;
StorageManager storageMgr; // Re-instantiate Storage Manager

// Global Server IP (Default fallback)
char serverIP[16] = "192.168.58.24";
bool serverIpUpdated = false; // Flag to track if IP was received via MQTT

// Command flags
bool shouldCapture = false;
// volatile bool isStreaming = false; // REMOVED: Defined in stream_manager.cpp
// volatile bool pauseStreamForCapture = false; // REMOVED: Defined in config.cpp
// volatile bool captureRequested = false; // REMOVED: Defined in config.cpp

// Forward declaration
void processCapture(camera_fb_t* fb);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String msg = "";
    for (int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }
    Serial.printf("📩 MQTT Message [%s]: %s\n", topic, msg.c_str());

    // Handle Server IP Discovery
    if (String(topic) == "camera/server-ip") {
        // Parse JSON: {"ip":"192.168.x.x", ...}
        int ipStart = msg.indexOf("\"ip\":\"");
        if (ipStart != -1) {
            int ipEnd = msg.indexOf("\"", ipStart + 6);
            String newIP = msg.substring(ipStart + 6, ipEnd);
            Serial.printf("📡 Received Server IP from MQTT: %s\n", newIP.c_str());
            strcpy(serverIP, newIP.c_str());
            serverIpUpdated = true; // Mark as updated
        }
        return;
    }

    if (String(topic) == MQTT_TOPIC_COMMAND) {
        if (msg == "capture") {
            Serial.println("📸 Command: CAPTURE");
            shouldCapture = true;
        } else if (msg == "stream_on") {
            Serial.println("🎥 Command: STREAM ON");
            isStreaming = true;
            if (!cameraMgr.isInitialized()) {
                cameraMgr.init();
            }
        } else if (msg == "stream_off") {
            Serial.println("🎥 Command: STREAM OFF");
            isStreaming = false;
            // Only deinit if not capturing
            if (!shouldCapture) {
                cameraMgr.deinit();
            }
        } else if (msg == "reboot") {
            ESP.restart();
        } else if (msg == "sync_sd") {
            Serial.println("🔄 Command: SYNC SD CARD");
            // Trigger sync logic
            if (storageMgr.isReady()) {
                size_t flushed = storageMgr.flushPendingQueue(authMgr.getToken(), uploadMgr);
                Serial.printf("Synced %d files\n", flushed);
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=================================");
    Serial.println("ESP32 Always-On Camera Starting");
    Serial.println("=================================");

    // 1. Init Camera - DEFERRED (Save Power)
    // Serial.println("[1/4] Initializing camera...");
    // if (!cameraMgr.init()) { ... }
    Serial.println("[1/4] Camera Init Deferred (Waiting for Command/Motion)");

    // 1.5 Init Status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW); // Ensure off initially
    
    // 2. Connect WiFi
    Serial.println("[2/4] Connecting to WiFi...");
    if (!wifiMgr.connect()) {
        Serial.println("[ERR] WiFi failed");
        ledMgr.flashRed(5); // WiFi thất bại
        // Retry logic could be added here
    } else {
        ledMgr.flashBlue(2); // WiFi thành công
    }

    // 3. MQTT Setup & Discovery (PRIORITY)
    // Connect MQTT first to get Server IP
    Serial.println("[3/4] Connecting to MQTT & Discovering Server...");
    if (USE_MQTT) {
        mqttMgr.setCallback(mqttCallback);
        if (mqttMgr.connect()) {
             Serial.println("✅ MQTT Connected. Waiting for Server IP...");
             // Wait for IP from 'camera/server-ip' (handled in callback)
             unsigned long startWait = millis();
             while (millis() - startWait < 3000 && !serverIpUpdated) {
                 mqttMgr.loop(); // Process incoming messages
                 delay(100);
             }
        } else {
             Serial.println("[WARN] MQTT connect failed");
        }
    }

    // Fallback to mDNS if MQTT didn't update IP
    if (!serverIpUpdated) {
        Serial.println("⚠️ MQTT Discovery timed out or failed. Trying mDNS...");
        if (MDNS.begin("esp32-cam")) {
            Serial.println("[mDNS] Responder started");
        }
        Serial.printf("[mDNS] Resolving %s (Timeout 5s)...\n", SERVER_HOSTNAME_MDNS);
        IPAddress ip = MDNS.queryHost(SERVER_HOSTNAME_MDNS, 5000);
        if (ip != IPAddress()) {
            Serial.printf("[mDNS] Resolved: %s\n", ip.toString().c_str());
            strcpy(serverIP, ip.toString().c_str());
        } else {
            Serial.println("[mDNS] No response (Timeout) - Check Firewall/Network");
            Serial.printf("[mDNS] Using Fallback IP: %s\n", serverIP);
        }
    } else {
        Serial.printf("✅ Server IP updated via MQTT: %s\n", serverIP);
    }

    // 4. Auth Setup (Now we have the correct IP)
    Serial.println("[4/4] Authenticating...");
    if (!authMgr.ensureLoggedIn()) {
        Serial.println("[WARN] Auth failed - uploads might fail");
        ledMgr.flashRed(2); // Auth thất bại
    } else {
        ledMgr.flashYellow(2); // Auth thành công
    }

    // 5. Start Stream Server
    Serial.println("[5/5] Starting Stream Server...");
    streamMgr.setCaptureCallback(processCapture); // Register callback
    streamMgr.startWebServer();
    Serial.print("Stream Ready at http://");
    Serial.print(WiFi.localIP());
    Serial.printf(":%d/stream\n", STREAM_PORT);
    
    // 5. Init SD Card
    Serial.println("[5/5] Mounting SD Card...");
    if (storageMgr.begin()) {
        Serial.println("✅ SD Card Ready");
    } else {
        Serial.println("⚠️ SD Card Failed");
    }

    pinMode(PIR_PIN, INPUT_PULLDOWN);
    
    // Check initial state
    if (digitalRead(PIR_PIN) == HIGH) {
        Serial.println("⚠️ WARNING: PIR Pin is HIGH at startup!");
        Serial.println("   If no sensor is connected, your board has a Pull-Up resistor.");
        Serial.println("   Connect the pin to GND to stop auto-capture.");
        
        // Indicate warning with Orange LED (R=255, G=165, B=0)
        ledMgr.setStatusColor(255, 165, 0);
        delay(3000); // Hold orange for 3 seconds
    } else {
        ledMgr.flashGreen(3);
    }

    Serial.println("✅ System Ready. Loop started.");
}

// Extracted function to process a captured frame
void processCapture(camera_fb_t* fb) {
    if (!fb) return;
    
    Serial.println("🖼️ Processing captured frame...");
    ledMgr.flashWhite(1);
    
    // Always save to SD first (Backup)
    if (storageMgr.isReady()) {
        storageMgr.savePendingFrame(fb);
    }

    bool uploadSuccess = false;
    if (USE_MQTT && mqttMgr.isConnected()) {
        uploadSuccess = mqttMgr.publishImageChunked(fb->buf, fb->len);
    } else {
        // Fallback to HTTP
        uploadSuccess = uploadMgr.upload(fb, authMgr.getToken());
    }
    
    if (uploadSuccess) {
        Serial.println("✅ Upload complete");
        ledMgr.flashGreen(1); // Gửi ảnh thành công
        // Move file from 'pending' to 'sent' folder on SD
        if (storageMgr.isReady()) {
            storageMgr.moveToSent(storageMgr.getLastPath());
        }
    } else {
        Serial.println("❌ Upload failed - Saved to SD for later");
        ledMgr.flashRed(1); // Gửi ảnh thất bại
    }
}

void loop() {
    // 1. Maintain MQTT
    if (USE_MQTT) {
        // Prevent race condition: Don't run MQTT loop if Stream Task is uploading
        if (!captureRequested) {
            if (!mqttMgr.isConnected()) {
                 static unsigned long lastReconnectAttempt = 0;
                 unsigned long now = millis();
                 if (now - lastReconnectAttempt > 5000) {
                     lastReconnectAttempt = now;
                     if (mqttMgr.connect()) {
                         lastReconnectAttempt = 0;
                     }
                 }
            } else {
                mqttMgr.loop();
            }
        }
    }

    // 2. Check Motion
    static unsigned long lastMotionTime = 0;
    const unsigned long MOTION_COOLDOWN = 15000; // 15s cooldown

    bool motionDetected = false;

    if (USE_PIR) {
        // Software Debounce: Read twice to confirm signal is stable
        if (digitalRead(PIR_PIN) == HIGH) {
            delay(50); // Wait 50ms to filter noise spikes
            if (digitalRead(PIR_PIN) == HIGH) {
                motionDetected = true;
            }
        }
    }

    // Control External LED: ON when motion detected, OFF otherwise
    digitalWrite(STATUS_LED_PIN, motionDetected ? HIGH : LOW);
    
    if (motionDetected && (millis() - lastMotionTime > MOTION_COOLDOWN)) {
        Serial.println("🏃 Motion Detected (Stable Signal)!");
        shouldCapture = true;
        lastMotionTime = millis();
    }

    // 3. Handle Capture (from Motion or MQTT)
    if (shouldCapture) {
        shouldCapture = false;
        Serial.println("📸 Capture requested...");
        
        bool wasInitialized = cameraMgr.isInitialized();
        
        // If streaming, delegate capture to the stream task
        if (isStreaming && wasInitialized) {
            Serial.println("🔄 Delegating capture to Stream Task...");
            captureRequested = true;
            // We don't block here; the stream task will pick it up
        } else {
            // Standard capture flow (when not streaming)
            if (!wasInitialized) {
                if (!cameraMgr.init()) {
                    Serial.println("❌ Camera init failed for capture");
                    return;
                }
            }

            // Flash ON (Simulated with RGB)
            ledMgr.setFlash(true);
            delay(150); // Wait for light to stabilize

            camera_fb_t* fb = esp_camera_fb_get();
            
            // Flash OFF
            ledMgr.setFlash(false);

            if (fb) {
                processCapture(fb);
                esp_camera_fb_return(fb);
            } else {
                Serial.println("❌ Camera capture failed");
            }

            // Turn off camera if not streaming
            if (!wasInitialized && !isStreaming) {
                cameraMgr.deinit();
                Serial.println("💤 Camera de-initialized to save power");
            }
        }
    }

    delay(10); // Yield
}

