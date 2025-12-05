/**
 * led_manager.cpp - LED management implementation
 */

#include <Arduino.h>
#include "led_manager.h"

LEDManager::LEDManager() : _pixels(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800) {
}

void LEDManager::init() {
    _pixels.begin();
    clear();
}

uint8_t LEDManager::dim(uint8_t value) {
    // Giảm độ sáng xuống 30% (có thể chỉnh lại nếu muốn sáng hơn)
    return (uint8_t)(value * 0.3);
}

void LEDManager::flash(uint32_t ms, uint8_t r, uint8_t g, uint8_t b) {
    _pixels.setPixelColor(0, _pixels.Color(dim(r), dim(g), dim(b)));
    _pixels.show();
    delay(ms);
    clear();
}

void LEDManager::blinkError(int times) {
    for (int i = 0; i < times; i++) {
        flashRed(200);
        delay(200);
    }
}

void LEDManager::clear() {
    _pixels.clear();
    _pixels.show();
}

void LEDManager::flashGreen(int times) {
    for (int i = 0; i < times; i++) {
        flash(200, 0, 255, 0);
        if (i < times - 1) delay(200);
    }
}

void LEDManager::flashRed(int times) {
    for (int i = 0; i < times; i++) {
        flash(200, 255, 0, 0);
        if (i < times - 1) delay(200);
    }
}

void LEDManager::flashBlue(int times) {
    for (int i = 0; i < times; i++) {
        flash(300, 0, 0, 255);
        if (i < times - 1) delay(200);
    }
}

void LEDManager::flashWhite(int times) {
    for (int i = 0; i < times; i++) {
        flash(150, 255, 255, 255);
        if (i < times - 1) delay(150);
    }
}

void LEDManager::flashYellow(int times) {
    for (int i = 0; i < times; i++) {
        flash(200, 255, 255, 0);
        if (i < times - 1) delay(200);
    }
}

void LEDManager::setFlash(bool on) {
    if (on) {
        // Turn on full brightness white for camera flash
        _pixels.setPixelColor(0, _pixels.Color(255, 255, 255)); // Không dùng dim()
        _pixels.show();
    } else {
        clear();
    }
}

void LEDManager::setStatusColor(uint8_t r, uint8_t g, uint8_t b) {
    _pixels.setPixelColor(0, _pixels.Color(dim(r), dim(g), dim(b)));
    _pixels.show();
}

void LEDManager::showStatusColor(uint8_t r, uint8_t g, uint8_t b, uint16_t durationMs, bool hold) {
    setStatusColor(r, g, b);
    delay(durationMs);
    if (!hold) {
        clear();
    }
}

void LEDManager::gentlePulse(uint8_t r, uint8_t g, uint8_t b, uint8_t cycles, uint16_t stepDelayMs) {
    for (int c = 0; c < cycles; c++) {
        // Fade in
        for (int i = 0; i <= 100; i += 5) {
            float factor = i / 100.0;
            _pixels.setPixelColor(0, _pixels.Color(dim(r * factor), dim(g * factor), dim(b * factor)));
            _pixels.show();
            delay(stepDelayMs / 20);
        }
        // Fade out
        for (int i = 100; i >= 0; i -= 5) {
            float factor = i / 100.0;
            _pixels.setPixelColor(0, _pixels.Color(dim(r * factor), dim(g * factor), dim(b * factor)));
            _pixels.show();
            delay(stepDelayMs / 20);
        }
    }
    clear();
}

void LEDManager::indicateSdTransfer(uint8_t cycles) {
    for (int i = 0; i < cycles; i++) {
        flash(100, 255, 0, 255); // Magenta
        if (i < cycles - 1) delay(100);
    }
}

void LEDManager::setAwakeIndicator(bool on) {
    _awakeIndicatorOn = on;
    updateAuxLed(on);
}

void LEDManager::updateAuxLed(bool on) {
    // Placeholder for auxiliary LED control if needed
}

void LEDManager::captureFlash(uint16_t durationMs) {
    setFlash(true);
    delay(durationMs);
    setFlash(false);
}

void LEDManager::flashAmber(int times) {
    for (int i = 0; i < times; i++) {
        flash(200, 255, 191, 0);
        if (i < times - 1) delay(200);
    }
}
