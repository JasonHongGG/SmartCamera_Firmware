#pragma once
#include <Arduino.h>

class HttpHealthMonitor {
public:
    static void setup();
    static void recordError(const char* handler, esp_err_t error);
    static void recordSuccess(const char* handler);
    static bool shouldRestartServer();
    static void resetErrorCount();
    
private:
    static int totalErrors;
    static int consecutiveErrors;
    static unsigned long lastErrorTime;
    static unsigned long lastSuccessTime;
    static const int MAX_CONSECUTIVE_ERRORS = 10;
    static const int MAX_TOTAL_ERRORS = 50;
    static const unsigned long ERROR_RESET_INTERVAL = 300000; // 5分鐘
};