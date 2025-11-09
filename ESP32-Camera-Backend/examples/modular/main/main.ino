/**
 * main_mqtt.ino - Main ESP32 Deep Sleep Motion Camera with MQTT
 * 
 * Hardware:
 * - ESP32-S3-EYE
 * - PIR Sensor on GPIO14 (wake source)
 * - WS2812 LED on GPIO48
 * 
 * Features:
 * - Motion-triggered wake from deep sleep
 * - Auto-login with JWT token (cached in RTC memory)
 * - Camera capture + MQTT publish
 * - Fallback to HTTP if MQTT fails
 * - Ultra low power: ~10µA in sleep (~6 months on 3000mAh)
 */

#include "config.h"
#include "sleep_manager.h"
#include "wifi_manager.h"
#include "auth_manager.h"
#include "led_manager.h"
#include "camera_manager.h"
#include "mqtt_manager.h"
#include "upload_manager.h"

#if ENABLE_STREAM_SERVER
#include "stream_server.h"
#endif

// Manager instances
SleepManager sleepMgr;
WiFiManager wifiMgr;
AuthManager authMgr;
LEDManager ledMgr;
CameraManager cameraMgr;
MQTTManager mqttMgr(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID);
UploadManager uploadMgr;

#if ENABLE_STREAM_SERVER
StreamServer streamServer;
#endif

// Global state
bool captureSuccess = false;
camera_fb_t* fb = nullptr;

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n\n=================================");
    Serial.println("🚀 ESP32 Motion Camera Starting");
    Serial.println("   MODE: MQTT + HTTP Fallback");
    Serial.println("=================================");
    
    // Check wake reason
    if (sleepMgr.wokeByMotion()) {
        Serial.println("🏃 WAKE: Motion detected!");
        ledMgr.flashBlue(2); // Motion indicator
    } else {
        Serial.println("🔌 WAKE: Power-on or reset");
        ledMgr.flashWhite(1);
    }
    
    // Step 1: Connect to WiFi
    Serial.println("\n[1/6] Connecting to WiFi...");
    if (!wifiMgr.connect()) {
        Serial.println("❌ WiFi failed - sleeping 30s");
        ledMgr.flashRed(3);
        sleepMgr.enableTimerWake(30 * 1000000); // Retry in 30s
        sleepMgr.enterDeepSleep();
    }
    Serial.print("✅ Connected! IP: ");
    Serial.println(WiFi.localIP());
    ledMgr.flashGreen(1);
    
    // Step 2: Authenticate (still needed for HTTP fallback)
    Serial.println("\n[2/6] Authenticating...");
    if (!authMgr.ensureLoggedIn()) {
        Serial.println("❌ Auth failed - sleeping 30s");
        ledMgr.flashRed(3);
        sleepMgr.enableTimerWake(30 * 1000000);
        sleepMgr.enterDeepSleep();
    }
    Serial.println("✅ Authenticated!");
    ledMgr.flashGreen(1);
    
    // Step 3: Connect to MQTT (if enabled)
    bool mqttConnected = false;
    #if USE_MQTT
    Serial.println("\n[3/6] Connecting to MQTT...");
    mqttConnected = mqttMgr.connect();
    if (mqttConnected) {
        Serial.println("✅ MQTT connected!");
        ledMgr.flashGreen(1);
    } else {
        Serial.println("⚠️ MQTT failed - will use HTTP fallback");
        ledMgr.flashYellow(2);
    }
    #else
    Serial.println("\n[3/6] MQTT disabled - using HTTP only");
    #endif
    
    // Step 4: Initialize camera
    Serial.println("\n[4/6] Initializing camera...");
    if (!cameraMgr.init()) {
        Serial.println("❌ Camera init failed - sleeping");
        ledMgr.flashRed(5);
        sleepMgr.enterDeepSleep();
    }
    Serial.println("✅ Camera ready!");
    
    // Step 4.5: Start stream server if enabled
    #if ENABLE_STREAM_SERVER
    Serial.println("\n[4.5/6] Starting stream server...");
    if (streamServer.begin(STREAM_SERVER_PORT)) {
        Serial.println("✅ Stream server running!");
        Serial.print("   Stream URL: http://");
        Serial.print(WiFi.localIP());
        Serial.print(":");
        Serial.print(STREAM_SERVER_PORT);
        Serial.println("/stream");
    } else {
        Serial.println("⚠️ Stream server failed to start");
    }
    #endif
    
    // Step 5: Capture photo
    Serial.println("\n[5/6] Capturing photo...");
    ledMgr.setFlash(true); // Flash on
    delay(100); // Let flash stabilize
    
    fb = cameraMgr.capture();
    ledMgr.setFlash(false); // Flash off
    
    if (!fb) {
        Serial.println("❌ Capture failed - sleeping");
        ledMgr.flashRed(3);
        cameraMgr.deinit();
        sleepMgr.enterDeepSleep();
    }
    
    Serial.print("✅ Captured! Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
    ledMgr.flashGreen(2);
    
    // Step 6: Upload via MQTT or HTTP
    Serial.println("\n[6/6] Uploading image...");
    
    // Try MQTT first if connected
    if (mqttConnected) {
        Serial.println("📡 Attempting MQTT publish...");
        if (mqttMgr.publishImage(fb->buf, fb->len)) {
            Serial.println("✅ MQTT upload successful!");
            ledMgr.flashGreen(3);
            captureSuccess = true;
        } else {
            Serial.println("⚠️ MQTT publish failed - falling back to HTTP...");
            ledMgr.flashYellow(2);
            mqttConnected = false; // Force HTTP fallback
        }
    }
    
    // Fallback to HTTP if MQTT failed or not connected
    if (!captureSuccess && !mqttConnected) {
        Serial.println("📤 Using HTTP upload...");
        String token = authMgr.getToken();
        
        if (uploadMgr.uploadImage(fb->buf, fb->len, token)) {
            Serial.println("✅ HTTP upload successful!");
            ledMgr.flashGreen(3);
            captureSuccess = true;
        } else {
            Serial.println("❌ HTTP upload failed");
            ledMgr.flashRed(3);
        }
    }
    
    // Cleanup
    if (mqttConnected) {
        mqttMgr.disconnect();
    }
    cameraMgr.returnFrameBuffer(fb);
    
    #if !ENABLE_STREAM_SERVER
    // Only deinit camera if not streaming
    cameraMgr.deinit();
    wifiMgr.disconnect();
    #endif
    
    // Print summary
    Serial.println("\n=================================");
    Serial.println("📊 SUMMARY");
    Serial.println("=================================");
    Serial.print("Status: ");
    Serial.println(captureSuccess ? "SUCCESS ✅" : "FAILED ❌");
    Serial.print("Upload method: ");
    Serial.println(mqttConnected && captureSuccess ? "MQTT" : "HTTP");
    Serial.print("Wake reason: ");
    Serial.println(sleepMgr.wokeByMotion() ? "Motion" : "Other");
    
    #if ENABLE_STREAM_SERVER
    Serial.println("🎥 Stream server: ACTIVE");
    Serial.print("   Access at: http://");
    Serial.print(WiFi.localIP());
    Serial.print(":");
    Serial.print(STREAM_SERVER_PORT);
    Serial.println("/stream");
    #endif
    
    Serial.println("=================================");
    
    #if ENABLE_STREAM_SERVER
    // Stay awake for streaming
    Serial.println("\n♾️  Staying awake for streaming...");
    Serial.println("   (Deep sleep disabled when stream server is enabled)");
    #else
    // Enter deep sleep - will wake on next motion
    delay(500);
    sleepMgr.enterDeepSleep();
    #endif
}

void loop() {
    #if ENABLE_STREAM_SERVER
    // Keep stream server alive
    delay(100);
    
    // Optional: Add watchdog or reconnect logic here
    if (!WiFi.isConnected()) {
        Serial.println("⚠️ WiFi disconnected, reconnecting...");
        wifiMgr.connect();
    }
    #else
    // Never reaches here in normal operation - deep sleep doesn't return
    // If somehow reached, go back to sleep
    delay(1000);
    sleepMgr.enterDeepSleep();
    #endif
}
