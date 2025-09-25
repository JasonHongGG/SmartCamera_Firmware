#pragma once
#include <WiFi.h>

class WiFiManager {
private:
    // ===========================
    // Enter your WiFi credentials
    // ===========================
    // const char *ssid = "Hong";
    // const char *password = "789123852";
    const char *ssid = "SELAB";
    const char *password = "selab61035";

public:
    void wifiSetup();
    void reconnectWiFi();
    void checkWiFiConnection();
};

inline WiFiManager *WiFiMgr = new WiFiManager();