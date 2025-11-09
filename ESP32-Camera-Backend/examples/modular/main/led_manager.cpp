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

void LEDManager::flash(uint32_t ms, uint8_t r, uint8_t g, uint8_t b) {
    _pixels.setPixelColor(0, _pixels.Color(r, g, b));
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
        _pixels.setPixelColor(0, _pixels.Color(255, 255, 255));
        _pixels.show();
    } else {
        clear();
    }
}
