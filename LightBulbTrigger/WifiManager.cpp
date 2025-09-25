#include "WifiManager.h"

void WiFiManager::wifiSetup() {
    WiFi.begin(this->ssid, this->password);
    WiFi.setSleep(false);

    Serial.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("WiFi connected : ");
    Serial.println(WiFi.localIP());
}

void WiFiManager::reconnectWiFi() {
    Serial.println("WiFi disconnected. Attempting to reconnect...");
    WiFi.disconnect();
    wifiSetup();
}

void WiFiManager::checkWiFiConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        this->reconnectWiFi();
    }
}
