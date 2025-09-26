#include "WatchdogManager.h"

bool WatchdogManager::enabled = false;

void WatchdogManager::setup() {
    Serial.println("WatchdogManager::setup");
    
    // 配置任務看門狗 - 使用新的 API
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT * 1000, // 轉換為毫秒
        .idle_core_mask = 0,              // 不監控空閒任務
        .trigger_panic = true             // 觸發時重啟系統
    };
    
    esp_err_t err = esp_task_wdt_init(&wdt_config);
    if (err != ESP_OK) {
        Serial.printf("Watchdog init failed: %s\n", esp_err_to_name(err));
        return;
    }
    
    // 添加當前任務到看門狗
    err = esp_task_wdt_add(NULL);
    if (err != ESP_OK) {
        Serial.printf("Watchdog add task failed: %s\n", esp_err_to_name(err));
        return;
    }
    
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