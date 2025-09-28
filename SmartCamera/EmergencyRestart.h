#pragma once
#include <Arduino.h>

class EmergencyRestart {
public:
    static void setup();
    static void checkSystemHealth();
    static void triggerRestart(const char* reason);
    
private:
    static unsigned long lastHealthCheck;
    static int criticalErrorCount;
    static bool restartScheduled;
    static const unsigned long HEALTH_CHECK_INTERVAL = 60000; // 1分鐘
    static const int MAX_CRITICAL_ERRORS = 3;
};