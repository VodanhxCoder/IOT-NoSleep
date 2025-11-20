/**
 * led_manager.h - LED flash and status indicators
 */

#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

class LEDManager {
public:
    LEDManager();
    void init();
    void setAwakeIndicator(bool on);
    void flash(uint32_t ms, uint8_t r, uint8_t g, uint8_t b);
    void blinkError(int times);
    void clear();
    void setFlash(bool on);
    void captureFlash(uint16_t durationMs = FLASH_DURATION_MS);

    void setStatusColor(uint8_t r, uint8_t g, uint8_t b);
    void showStatusColor(uint8_t r, uint8_t g, uint8_t b, uint16_t durationMs = 300, bool hold = true);
    void gentlePulse(uint8_t r, uint8_t g, uint8_t b, uint8_t cycles = 1, uint16_t stepDelayMs = 40);
    void indicateSdTransfer(uint8_t cycles = 2);

    // Legacy helpers (now dimmed for softer transitions)
    void flashGreen(int times = 1);
    void flashRed(int times = 1);
    void flashBlue(int times = 1);
    void flashWhite(int times = 1);
    void flashYellow(int times = 1);
    void flashAmber(int times = 1);

private:
    Adafruit_NeoPixel _pixels;
    uint8_t dim(uint8_t value);
    bool _awakeIndicatorOn;
    void updateAuxLed(bool on);
};

#endif // LED_MANAGER_H
