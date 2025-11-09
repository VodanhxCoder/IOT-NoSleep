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
    void flash(uint32_t ms, uint8_t r, uint8_t g, uint8_t b);
    void blinkError(int times);
    void clear();
    void setFlash(bool on);  // Added: Camera flash control
    
    // Predefined colors
    void flashGreen(int times = 1);
    void flashRed(int times = 1);
    void flashBlue(int times = 1);
    void flashWhite(int times = 1);
    void flashYellow(int times = 1);

private:
    Adafruit_NeoPixel _pixels;
};

#endif // LED_MANAGER_H
