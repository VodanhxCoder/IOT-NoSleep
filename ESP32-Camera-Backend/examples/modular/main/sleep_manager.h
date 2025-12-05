

#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include "esp_sleep.h"
#include "config.h"

class SleepManager {
public:
    SleepManager();
    void enterDeepSleep();
    esp_sleep_wakeup_cause_t getWakeupCause();
    bool wokeByMotion();
    bool isMotionLineActive() const;
    
    // Optional: timer wake backup
    void enableTimerWake(uint64_t microseconds);

private:
    esp_sleep_wakeup_cause_t _wakeupCause;
};

#endif // SLEEP_MANAGER_H
