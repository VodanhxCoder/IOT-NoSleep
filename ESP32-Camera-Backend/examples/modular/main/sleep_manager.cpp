

#include <Arduino.h>
#include "sleep_manager.h"

SleepManager::SleepManager() {
    _wakeupCause = esp_sleep_get_wakeup_cause();
}

void SleepManager::enterDeepSleep() {
    Serial.println("💤 Entering deep sleep... (Wake on motion)");
    Serial.flush();
    
    // Configure ext0 wake on GPIO14 (PIR sensor)
    esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, 1); // Wake when HIGH
    
    // Enter deep sleep (RTC memory persists)
    esp_deep_sleep_start();
}

esp_sleep_wakeup_cause_t SleepManager::getWakeupCause() {
    return _wakeupCause;
}

bool SleepManager::wokeByMotion() {
    return _wakeupCause == ESP_SLEEP_WAKEUP_EXT0;
}

bool SleepManager::isMotionLineActive() const {
    return digitalRead(PIR_PIN) == HIGH;
}

void SleepManager::enableTimerWake(uint64_t microseconds) {
    esp_sleep_enable_timer_wakeup(microseconds);
    Serial.print("⏰ Timer wake enabled: ");
    Serial.print(microseconds / 1000000);
    Serial.println(" seconds");
}
