#include <TJpg_Decoder.h>
#include "App/app.cpp"
#include "WifiManager.h"
#include "CameraManager.h"
#include "FlashLightManager.h"




void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  FlashLightManager::setup();
  CameraMgr->start();
  WiFiMgr->wifiSetup();
  app->setup();
}

void loop() {
  WiFiMgr->checkWiFiConnection();
  delay(1000);
}
