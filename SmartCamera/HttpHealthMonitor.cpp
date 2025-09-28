#include "HttpHealthMonitor.h"

int HttpHealthMonitor::totalErrors = 0;
int HttpHealthMonitor::consecutiveErrors = 0;
unsigned long HttpHealthMonitor::lastErrorTime = 0;
unsigned long HttpHealthMonitor::lastSuccessTime = 0;

void HttpHealthMonitor::setup() {
    totalErrors = 0;
    consecutiveErrors = 0;
    lastErrorTime = 0;
    lastSuccessTime = millis();
    Serial.println("HttpHealthMonitor initialized");
}

void HttpHealthMonitor::recordError(const char* handler, esp_err_t error) {
    totalErrors++;
    consecutiveErrors++;
    lastErrorTime = millis();
    
    Serial.printf("HTTP Error in %s: %s (consecutive: %d, total: %d)\n", 
                  handler, esp_err_to_name(error), consecutiveErrors, totalErrors);
    
    // 如果連續錯誤太多，記錄嚴重警告
    if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS / 2) {
        Serial.printf("WARNING: High consecutive error count in %s: %d\n", handler, consecutiveErrors);
    }
}

void HttpHealthMonitor::recordSuccess(const char* handler) {
    consecutiveErrors = 0; // 重置連續錯誤計數
    lastSuccessTime = millis();
    
    // 偶爾輸出成功日誌
    static unsigned long lastSuccessLog = 0;
    if (millis() - lastSuccessLog > 60000) { // 每分鐘最多一次
        Serial.printf("HTTP Success in %s (total errors since last reset: %d)\n", handler, totalErrors);
        lastSuccessLog = millis();
    }
}

bool HttpHealthMonitor::shouldRestartServer() {
    unsigned long currentTime = millis();
    
    // 檢查是否連續錯誤過多
    if (consecutiveErrors >= MAX_CONSECUTIVE_ERRORS) {
        Serial.printf("Server restart triggered: consecutive errors (%d) exceeded limit\n", consecutiveErrors);
        return true;
    }
    
    // 檢查總錯誤數是否過多
    if (totalErrors >= MAX_TOTAL_ERRORS) {
        Serial.printf("Server restart triggered: total errors (%d) exceeded limit\n", totalErrors);
        return true;
    }
    
    // 檢查是否長時間沒有成功的請求
    if (currentTime - lastSuccessTime > 120000 && totalErrors > 0) { // 2分鐘沒有成功請求
        Serial.printf("Server restart triggered: no successful requests for %lu ms\n", 
                     currentTime - lastSuccessTime);
        return true;
    }
    
    return false;
}

void HttpHealthMonitor::resetErrorCount() {
    Serial.printf("Resetting error counts - was: consecutive=%d, total=%d\n", 
                  consecutiveErrors, totalErrors);
    totalErrors = 0;
    consecutiveErrors = 0;
}