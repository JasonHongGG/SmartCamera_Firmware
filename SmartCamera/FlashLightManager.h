#pragma once
#include <Arduino.h>
#include "CameraPins.h"
#define CONFIG_LED_MAX_INTENSITY 255

class FlashLightManager {
public:
    static int led_duty;
    static bool isStreaming;
    static void setup();
    static void openFlashLight(bool on);
};
