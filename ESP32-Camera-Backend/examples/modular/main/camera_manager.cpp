/**
 * camera_manager.cpp - Camera management implementation
 */

#include <Arduino.h>
#include "camera_manager.h"

CameraManager::CameraManager() {
    _initialized = false;
}

bool CameraManager::init() {
    Serial.println("Initializing camera...");
    
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    
    // Quality settings - Check PSRAM
    if (ESP.getPsramSize() > 0) {
        config.frame_size = FRAME_SIZE_HIGH;
        config.jpeg_quality = JPEG_QUALITY_HIGH;
        config.fb_count = FB_COUNT_HIGH;
        config.grab_mode = CAMERA_GRAB_LATEST;
        Serial.println("PSRAM found - High quality mode");
    } else {
        config.frame_size = FRAME_SIZE_STD;
        config.jpeg_quality = JPEG_QUALITY_STD;
        config.fb_count = FB_COUNT_STD;
        config.fb_location = CAMERA_FB_IN_DRAM;
        Serial.println("No PSRAM - Standard quality mode");
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("✗ Camera init failed: 0x%x\n", err);
        return false;
    }
    
    configureSensor();
    _initialized = true;
    Serial.println("✓ Camera ready");
    return true;
}

void CameraManager::configureSensor() {
    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 0);
        s->set_contrast(s, 0);
        s->set_saturation(s, 0);
        s->set_vflip(s, 0);
        s->set_hmirror(s, 0);
    }
}

void CameraManager::deinit() {
    if (_initialized) {
        esp_camera_deinit();
        _initialized = false;
        Serial.println("Camera deinitialized");
    }
}

camera_fb_t* CameraManager::capture() {
    if (!_initialized) {
        Serial.println("✗ Camera not initialized");
        return nullptr;
    }
    
    Serial.println("📸 Capturing image...");
    camera_fb_t *fb = esp_camera_fb_get();
    
    if (!fb) {
        Serial.println("✗ Capture failed");
        return nullptr;
    }
    
    Serial.printf("Image: %u bytes (%dx%d)\n", fb->len, fb->width, fb->height);
    return fb;
}

void CameraManager::returnFrameBuffer(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool CameraManager::isInitialized() {
    return _initialized;
}
