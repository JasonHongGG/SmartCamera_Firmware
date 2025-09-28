#include <TJpg_Decoder.h>
#include "App/app.cpp"
#include "WifiManager.h"
#include "CameraManager.h"
#include "FlashLightManager.h"
#include "WatchdogManager.h"
#include "HttpHealthMonitor.h"
#include "EmergencyRestart.h"
#include "EmergencyRestart.h"




void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // 啟動看門狗
  WatchdogManager::setup();
  
  // 啟動緊急重啟系統
  EmergencyRestart::setup();
  
  FlashLightManager::setup();
  
  //啟動相機
  delay(1000);
  Serial.println("Starting camera...");
  if (!CameraMgr->start()) {
    Serial.println("Initial camera start failed, will retry later");
  }
    
  WiFiMgr->wifiSetup();
  app->setup();
  
  Serial.println("=== System initialization completed ===");
}

void loop() {
  // 餵看門狗 - 這是最重要的，防止系統掛死
  WatchdogManager::feed();
  
  // 檢查 WiFi 連接
  WiFiMgr->checkWiFiConnection();
  
  // 檢查 HTTP 服務器健康狀態
  app->checkServerHealth();
  
  // 檢查緊急重啟條件
  EmergencyRestart::checkSystemHealth();
  
  // 如果相機未初始化，嘗試重新啟動
  static unsigned long lastCameraRetry = 0;
  if (!CameraMgr->isInitialized() && (millis() - lastCameraRetry > 10000)) {
    lastCameraRetry = millis();
    Serial.println("Camera not initialized, attempting restart...");
    CameraMgr->start();
  }
  
  delay(1000);
}
