#include "FlashLightManager.h"

int FlashLightManager::led_duty = 0;
bool FlashLightManager::isStreaming = false;
int FlashLightManager::lastDuty = -1; // 初始化為 -1 表示未設置

void FlashLightManager::setup() {
  ledcAttach(FLASH_LED_PIN, 5000, 8);
}

void FlashLightManager::openFlashLight(bool en) {  // Turn LED On or Off
  int duty = en ? led_duty : 0;
  if (en && FlashLightManager::isStreaming && (led_duty > CONFIG_LED_MAX_INTENSITY)) {
    duty = CONFIG_LED_MAX_INTENSITY;
  }
  
  // 只有當亮度值真的改變時才設置和輸出訊息
  if (duty != lastDuty) {
    ledcWrite(FLASH_LED_PIN, duty);
    Serial.printf("Set LED intensity to %d\n", duty);
    lastDuty = duty;
  }
}