#include "BulbServer.h"

void BulbServer::bulbOpen(bool state) {
    if (state) {
      digitalWrite(RELAY_PIN, HIGH);  // 開啟繼電器
      Serial.println("Bulb is ON");
    } else {
      digitalWrite(RELAY_PIN, LOW);   // 關閉繼電器
      Serial.println("Bulb is OFF");
    }
}

void BulbServer::handleRoot() {
    server.send(200, "text/plain", "ESP32 HTTP Server 正常運行");
}

void BulbServer::handleControl() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        if (body.indexOf("ON") >= 0)  {
            deviceStatus = "ON";
            bulbOpen(true);
        }
        else if (body.indexOf("OFF") >= 0) {
            deviceStatus = "OFF";
            bulbOpen(false);
        }
        server.send(200, "application/json", "{\"message\":\"狀態已更新\",\"status\":\"" + deviceStatus + "\"}");
    } else {
        server.send(400, "application/json", "{\"error\":\"沒有收到資料\"}");
    }
}

void BulbServer::begin() { 
    server.begin(); 
}

void BulbServer::handleClient() { 
    server.handleClient(); 
}