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
#include "sleep_manager.h"
#include "wifi_manager.h"
#include "auth_manager.h"
#include "led_manager.h"
#include "camera_manager.h"
#include "upload_manager.h"
#include "storage_manager.h"

// Manager instances
SleepManager sleepMgr;
WiFiManager wifiMgr;
AuthManager authMgr;
LEDManager ledMgr;
CameraManager cameraMgr;
UploadManager uploadMgr;
StorageManager storageMgr;

// Global state
bool captureSuccess = false;
camera_fb_t* fb = nullptr;

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n\n=================================");
    Serial.println("ESP32 Motion Camera Starting");
    Serial.println("=================================");

    // Check wake reason
    if (sleepMgr.wokeByMotion()) {
        Serial.println("[WAKE] Motion detected!");
        ledMgr.flashBlue(2); // Motion indicator
    } else {
        Serial.println("[WAKE] Power-on or reset");
        ledMgr.flashWhite(1);
    }

    // Step 0: Mount SD card (optional offline queue)
    Serial.println("\n[0/5] Mounting SD card...");
    bool sdReady = storageMgr.begin();

    // Step 1: Initialize camera and capture immediately
    Serial.println("\n[1/5] Initializing camera...");
    if (!cameraMgr.init()) {
        Serial.println("[ERR] Camera init failed - sleeping");
        ledMgr.flashRed(5);
        sleepMgr.enterDeepSleep();
    }
    Serial.println("[OK] Camera ready!");

    Serial.println("\n[2/5] Capturing photo...");
    ledMgr.setFlash(true);
    delay(100);
    fb = cameraMgr.capture();
    ledMgr.setFlash(false);

    if (!fb) {
        Serial.println("[ERR] Capture failed - sleeping");
        ledMgr.flashRed(3);
        cameraMgr.deinit();
        sleepMgr.enterDeepSleep();
    }

    Serial.print("[OK] Captured! Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
    ledMgr.flashGreen(2);

    // Step 2: Connect to WiFi
    Serial.println("\n[3/5] Connecting to WiFi...");
    if (!wifiMgr.connect()) {
        Serial.println("[ERR] WiFi failed - storing frame for later");
        ledMgr.flashRed(3);
        if (sdReady && fb) {
            storageMgr.savePendingFrame(fb);
        }
        cameraMgr.returnFrameBuffer(fb);
        fb = nullptr;
        cameraMgr.deinit();
        sleepMgr.enableTimerWake(30 * 1000000);
        sleepMgr.enterDeepSleep();
    }
    Serial.print("[OK] Connected! IP: ");
    Serial.println(WiFi.localIP());
    ledMgr.flashGreen(1);

    // Step 3: Authenticate
    Serial.println("\n[4/5] Authenticating...");
    if (!authMgr.ensureLoggedIn()) {
        Serial.println("[ERR] Auth failed - storing frame for later");
        ledMgr.flashRed(3);
        if (sdReady && fb) {
            storageMgr.savePendingFrame(fb);
        }
        cameraMgr.returnFrameBuffer(fb);
        fb = nullptr;
        cameraMgr.deinit();
        sleepMgr.enableTimerWake(30 * 1000000);
        sleepMgr.enterDeepSleep();
    }
    Serial.println("[OK] Authenticated!");
    ledMgr.flashGreen(1);
    String token = authMgr.getToken();

    // Step 5: Upload to server
    Serial.println("\n[5/5] Uploading to server...");

    if (uploadMgr.uploadImage(fb->buf, fb->len, token)) {
        Serial.println("[OK] Upload successful!");
        ledMgr.flashGreen(3);
        captureSuccess = true;
    } else {
        Serial.println("[ERR] Upload failed");
        ledMgr.flashRed(3);
        if (sdReady) {
            storageMgr.savePendingFrame(fb);
        }
    }

    // Cleanup
    cameraMgr.returnFrameBuffer(fb);
    cameraMgr.deinit();

    if (sdReady) {
        Serial.println("\n[PENDING] Flushing queued captures...");
        size_t flushed = storageMgr.flushPendingQueue(token, uploadMgr);
        if (flushed > 0) {
            Serial.printf("[PENDING] Uploaded %u pending file(s)\n", (unsigned)flushed);
            captureSuccess = true;
        } else {
            Serial.println("[PENDING] No pending files to upload");
        }
    }

    wifiMgr.disconnect();

    // Print summary
    Serial.println("\n=================================");
    Serial.println("SUMMARY");
    Serial.println("=================================");
    Serial.print("Status: ");
    Serial.println(captureSuccess ? "SUCCESS" : "FAILED");
    Serial.print("Wake reason: ");
    Serial.println(sleepMgr.wokeByMotion() ? "Motion" : "Other");
    Serial.println("=================================");

    // Enter deep sleep - will wake on next motion
    delay(500);
    sleepMgr.enterDeepSleep();
}

void loop() {
    // Never reaches here - deep sleep doesn't return
    // If somehow reached, go back to sleep
    delay(1000);
    sleepMgr.enterDeepSleep();
}
