#include <TJpg_Decoder.h>
#include "App/app.cpp"
#include "WifiManager.h"
#include "CameraManager.h"
#include "FlashLightManager.h"
#include "WatchdogManager.h"




void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  // 啟動看門狗
  WatchdogManager::setup();
  
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
  delay(1000);
}
