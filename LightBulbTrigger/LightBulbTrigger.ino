#include "WifiManager.h"
#include "BulbServer.h"

void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    pinMode(RELAY_PIN, OUTPUT);
    WiFiMgr->wifiSetup();

    BulbServerInstance->begin();
    Serial.println("TCP Server started");
}

void loop() {
    WiFiMgr->checkWiFiConnection();
    BulbServerInstance->handleClient();
    delay(1000);
}