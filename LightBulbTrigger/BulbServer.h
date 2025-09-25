#pragma once
#include <WebServer.h>
#define RELAY_PIN 15  // 接到繼電器 IN 腳的 GPIO

class BulbServer {
public:
    BulbServer() : server(80) {
        server.on("/", [this]() { this->handleRoot(); });
        server.on("/open", HTTP_POST, [this]() { this->handleControl(); });
    }

    void bulbOpen(bool state);
    void handleRoot();
    void handleControl();
    void begin();
    void handleClient();

private:
    WebServer server;
    String deviceStatus = "OFF";
};

inline BulbServer *BulbServerInstance = new BulbServer();