#include "LightBulbManager.h"

void LightBulbManager::open(bool on) {
    Serial.println("LightBulbManager Trigger");
    String postUrl = serverBaseURL + "/open";
    http.begin(postUrl);
    http.addHeader("Content-Type", "application/json");
    int postCode;
    
    light_bulb = on ? 1 : 0;
    if (on) {
        postCode = http.POST("{\"command\":\"ON\"}");
    } else {
        postCode = http.POST("{\"command\":\"OFF\"}");
    }


    if (postCode > 0) {
        Serial.printf("HTTP POST 狀態碼: %d\n", postCode);
        Serial.println(http.getString());
    }
    http.end();
}