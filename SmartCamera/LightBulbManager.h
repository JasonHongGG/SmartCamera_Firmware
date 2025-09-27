#pragma once
#include <Arduino.h>
#include <HTTPClient.h>

class LightBulbManager {
public:
    bool light_bulb = false;
private:
    const char* SERVER_IP = "192.168.50.205"; // 改成 Server ESP32 的 IP
    String serverBaseURL = "http://" + String(SERVER_IP);
    HTTPClient http;
public:
    void open(bool on);
};

inline LightBulbManager *LightBulbMgr = new LightBulbManager();