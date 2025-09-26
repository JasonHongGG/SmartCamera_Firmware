#pragma once
#include <Arduino.h>
#include "esp_task_wdt.h"

class WatchdogManager {
public:
    static void setup();
    static void feed();
    static void enable();
    static void disable();
    
private:
    static bool enabled;
    static const uint32_t WDT_TIMEOUT = 30; // 30秒超時
};