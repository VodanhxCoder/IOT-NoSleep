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
 * - Ultra low power: ~10uA in sleep (~6 months on 3000mAh)
 */

#include "config.h"
#include "sleep_manager.h"
#include "wifi_manager.h"
#include "auth_manager.h"
#include "led_manager.h"
#include "camera_manager.h"
#include "mqtt_manager.h"
#include "upload_manager.h"
#include "storage_manager.h"
#include "encryption_manager.h"
#include "server_resolver.h"

// Manager instances
SleepManager sleepMgr;
WiFiManager wifiMgr;
AuthManager authMgr;
LEDManager ledMgr;
CameraManager cameraMgr;
MQTTManager mqttMgr(MQTT_BROKER, MQTT_PORT, MQTT_CLIENT_ID);
UploadManager uploadMgr;
StorageManager storageMgr;
EncryptionManager encryptionMgr;

// Global state
bool captureSuccess = false;
camera_fb_t* fb = nullptr;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=================================");
    Serial.println("dYs? ESP32 Motion Camera Starting");
    Serial.println("   MODE: MQTT + HTTP Fallback");
    Serial.println("=================================");

    // Check wake reason
    if (sleepMgr.wokeByMotion()) {
        Serial.println("dY? WAKE: Motion detected!");
        ledMgr.flashBlue(2); // Motion indicator
    } else {
        Serial.println("dY\"O WAKE: Power-on or reset");
        ledMgr.flashWhite(1);
    }

    // Step 0: Mount SD card (optional offline queue)
    Serial.println("\n[0/6] Preparing SD queue...");
    storageMgr.begin();
    ledMgr.showStatusColor(80, 80, 80, 300);

    // Step 1: Connect to WiFi
    Serial.println("\n[1/6] Connecting to WiFi...");
    ledMgr.showStatusColor(255, 180, 0, 400); // amber = searching WiFi
    if (!wifiMgr.connect()) {
        Serial.println("WiFi failed - sleeping 30s");
        ledMgr.flashRed(3);
        sleepMgr.enableTimerWake(30 * 1000000);
        sleepMgr.enterDeepSleep();
    }
    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
    ledMgr.showStatusColor(0, 150, 0, 400); // soft green = WiFi OK

    // Discover backend host via mDNS/DNS
    Serial.println("\n[1.5/6] Discovering backend host...");
    bool mdnsResolved = serverResolver.resolve();
    if (mdnsResolved) {
        Serial.print("[NET] Connection locked to desktop via mDNS: ");
    } else {
        Serial.print("[NET] Fallback connection using IP: ");
    }
    Serial.println(serverResolver.baseUrl());
    ledMgr.showStatusColor(mdnsResolved ? 0 : 255,
                           mdnsResolved ? 120 : 40,
                           mdnsResolved ? 255 : 0,
                           450);
    if (!mdnsResolved) {
        Serial.println("[NET] Using fallback IP for API + MQTT");
    }
#if USE_MQTT
    mqttMgr.updateBroker(serverResolver.mqttHost());
#endif

    // Step 2: Authenticate (still needed for HTTP fallback)
    Serial.println("\n[2/6] Authenticating...");
    if (!authMgr.ensureLoggedIn()) {
        Serial.println("Auth failed - sleeping 30s");
        ledMgr.flashRed(3);
        sleepMgr.enableTimerWake(30 * 1000000);
        sleepMgr.enterDeepSleep();
    }
    Serial.println("Authenticated!");
    ledMgr.showStatusColor(0, 90, 255, 400); // blue = auth OK

    if (storageMgr.isReady()) {
        storageMgr.flushPendingQueue(authMgr.getToken(), uploadMgr);
    }

    // Step 3: Connect to MQTT (if enabled)
    bool mqttConnected = false;
    bool uploadedViaMqtt = false;
#if USE_MQTT
    Serial.println("\n[3/6] Connecting to MQTT...");
    mqttConnected = mqttMgr.connect();
    if (mqttConnected) {
        Serial.println("MQTT connected");
        ledMgr.flashGreen(1);
    } else {
        Serial.println("MQTT failed - will use HTTP fallback");
        ledMgr.flashYellow(2);
    }
#else
    Serial.println("\n[3/6] MQTT disabled - using HTTP only");
#endif

    // Step 4: Initialize camera
    Serial.println("\n[4/6] Initializing camera...");
    if (!cameraMgr.init()) {
        Serial.println("Camera init failed - sleeping");
        ledMgr.flashRed(5);
        sleepMgr.enterDeepSleep();
    }
    Serial.println("Camera ready!");

    // Step 5: Capture photo
    Serial.println("\n[5/6] Capturing photo...");
    ledMgr.setFlash(true);
    delay(100);

    fb = cameraMgr.capture();
    size_t frameSizeBytes = fb ? fb->len : 0;
    ledMgr.setFlash(false);

    if (!fb) {
        Serial.println("Capture failed - sleeping");
        ledMgr.flashRed(3);
        cameraMgr.deinit();
        sleepMgr.enterDeepSleep();
    }

    Serial.print("Captured! Size: ");
    Serial.print(frameSizeBytes);
    Serial.println(" bytes");
    ledMgr.flashGreen(2);

    // Step 6: Upload via MQTT or HTTP
    Serial.println("\n[6/6] Uploading image...");

#if USE_MQTT && MQTT_FOR_IMAGES
    if (mqttConnected) {
        Serial.println("Attempting MQTT publish...");
        if (mqttMgr.publishImage(fb->buf, fb->len)) {
            Serial.println("MQTT upload successful!");
            ledMgr.flashGreen(3);
            captureSuccess = true;
            uploadedViaMqtt = true;
        } else {
            Serial.println("MQTT publish failed - falling back to HTTP");
            ledMgr.flashYellow(2);
            mqttConnected = false;
        }
    }
#endif

    bool shouldUseHttp =
#if USE_MQTT && MQTT_FOR_IMAGES
        (!captureSuccess && !mqttConnected);
#else
        (!captureSuccess);
#endif

    if (shouldUseHttp) {
        Serial.println("Using HTTP upload...");
        String token = authMgr.getToken();

        EncryptionResult encResult;
        if (!encryptionMgr.encrypt(fb->buf, fb->len, encResult)) {
            Serial.println("Encryption failed");
            ledMgr.flashRed(3);
        } else {
            if (uploadMgr.uploadImage(encResult.data, encResult.length, token, String(encResult.ivBase64))) {
                Serial.println("HTTP upload successful!");
                ledMgr.flashGreen(3);
                captureSuccess = true;
            } else {
                Serial.println("HTTP upload failed");
                ledMgr.flashRed(3);
                if (storageMgr.isReady()) {
                    storageMgr.savePendingFrame(fb);
                }
            }
            encryptionMgr.freeResult(encResult);
        }
    }

    if (mqttConnected) {
        mqttMgr.disconnect();
    }
    cameraMgr.returnFrameBuffer(fb);
    fb = nullptr;
    cameraMgr.deinit();
    wifiMgr.disconnect();

    Serial.println("\n=================================");
    Serial.println("dY\"S SUMMARY");
    Serial.println("=================================");
    Serial.print("Status: ");
    Serial.println(captureSuccess ? "SUCCESS" : "FAILED");
    Serial.print("Upload method: ");
    Serial.println(uploadedViaMqtt ? "MQTT" : "HTTP");
    Serial.print("Wake reason: ");
    Serial.println(sleepMgr.wokeByMotion() ? "Motion" : "Other");
    Serial.println("=================================");

#if USE_MQTT
    String statusPayload = String("{\"event\":\"upload\",\"result\":\"") +
        (captureSuccess ? "success" : "failure") + "\",\"size\":" + frameSizeBytes + "}";
    mqttMgr.publishStatus(statusPayload.c_str());
#endif

    delay(500);
    sleepMgr.enterDeepSleep();
}

void loop() {
    delay(1000);
    sleepMgr.enterDeepSleep();
}
