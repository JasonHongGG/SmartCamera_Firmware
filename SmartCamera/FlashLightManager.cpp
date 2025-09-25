#include "FlashLightManager.h"

int FlashLightManager::led_duty = 0;
bool FlashLightManager::isStreaming = false;

void FlashLightManager::setup() {
  ledcAttach(FLASH_LED_PIN, 5000, 8);
}

void FlashLightManager::openFlashLight(bool en) {  // Turn LED On or Off
  int duty = en ? led_duty : 0;
  if (en && FlashLightManager::isStreaming && (led_duty > CONFIG_LED_MAX_INTENSITY)) {
    duty = CONFIG_LED_MAX_INTENSITY;
  }
  ledcWrite(FLASH_LED_PIN, duty);
  Serial.printf("Set LED intensity to %d\n", duty);
}