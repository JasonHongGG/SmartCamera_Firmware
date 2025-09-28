#include "EmergencyRestart.h"
#include <WiFi.h>

unsigned long EmergencyRestart::lastHealthCheck = 0;
int EmergencyRestart::criticalErrorCount = 0;
bool EmergencyRestart::restartScheduled = false;

void EmergencyRestart::setup() {
    lastHealthCheck = millis();
    criticalErrorCount = 0;
    restartScheduled = false;
    Serial.println("Emergency restart system initialized");
}

void EmergencyRestart::checkSystemHealth() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastHealthCheck < HEALTH_CHECK_INTERVAL) {
        return;
    }
    
    lastHealthCheck = currentTime;
    
    // 檢查記憶體是否嚴重不足
    uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 5000) { // 5KB 以下為危險狀態
        criticalErrorCount++;
        Serial.printf("CRITICAL: Extremely low memory: %u bytes (error count: %d)\n", 
                     freeHeap, criticalErrorCount);
        
        if (criticalErrorCount >= MAX_CRITICAL_ERRORS) {
            triggerRestart("Critical memory shortage");
        }
    } else if (freeHeap > 20000) {
        // 記憶體恢復正常，重置錯誤計數
        if (criticalErrorCount > 0) {
            Serial.printf("Memory recovered: %u bytes, resetting error count\n", freeHeap);
            criticalErrorCount = 0;
        }
    }
    
    // 檢查系統是否響應正常
    static unsigned long lastWiFiCheck = 0;
    if (WiFi.status() != WL_CONNECTED && currentTime - lastWiFiCheck > 30000) {
        lastWiFiCheck = currentTime;
        Serial.println("WARNING: WiFi disconnected for extended period");
        // WiFi 斷線不立即重啟，但記錄警告
    }
}

void EmergencyRestart::triggerRestart(const char* reason) {
    if (restartScheduled) {
        return; // 避免重複觸發
    }
    
    restartScheduled = true;
    
    Serial.println("=========================================");
    Serial.printf("EMERGENCY RESTART TRIGGERED: %s\n", reason);
    Serial.println("System will restart in 5 seconds...");
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Uptime: %lu ms\n", millis());
    Serial.println("=========================================");
    
    // 給系統時間完成當前操作並輸出日誌
    for (int i = 5; i > 0; i--) {
        Serial.printf("Restarting in %d seconds...\n", i);
        delay(1000);
    }
    
    Serial.println("Restarting now...");
    ESP.restart();
}