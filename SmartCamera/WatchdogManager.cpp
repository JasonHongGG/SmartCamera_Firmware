#include "WatchdogManager.h"

bool WatchdogManager::enabled = false;

void WatchdogManager::setup() {
    Serial.println("WatchdogManager::setup");
    
    // 配置任務看門狗
    esp_task_wdt_init(WDT_TIMEOUT, true); // 30秒超時，自動重啟
    esp_task_wdt_add(NULL); // 添加當前任務到看門狗
    
    enabled = true;
    Serial.printf("Watchdog enabled with %d second timeout\n", WDT_TIMEOUT);
}

void WatchdogManager::feed() {
    if (enabled) {
        esp_task_wdt_reset(); // 餵狗
    }
}

void WatchdogManager::enable() {
    if (!enabled) {
        setup();
    }
}

void WatchdogManager::disable() {
    if (enabled) {
        esp_task_wdt_delete(NULL);
        esp_task_wdt_deinit();
        enabled = false;
        Serial.println("Watchdog disabled");
    }
}