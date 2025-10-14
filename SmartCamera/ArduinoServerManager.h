# pragma once
#include "WifiManager.h"
#include "LightBulbManager.h"
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// 前向聲明
class ArduinoServerManager;
extern ArduinoServerManager *ArduinoServerMgr;

class ArduinoServerManager {
public:
    CloudSwitch lightSwitch;
    static constexpr const char* DEVICE_KEY = "xYv9#1xW13lrSMjrrVVjIl90K";
    static constexpr const char* DEVICE_LOGIN_NAME = "4e2a1e1b-e0c8-40b3-a902-53f1dae42818";
    
private:
    WiFiConnectionHandler* connectionHandler = nullptr;
    
public:

    ArduinoServerManager() {
        // 不在建構函數中初始化，避免生命週期問題
    }
    
    void begin() {
        // 確保 WiFi 已經連接
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected, skipping Arduino IoT Cloud setup");
            return;
        }
        connectionHandler = new WiFiConnectionHandler(WiFiMgr->ssid, WiFiMgr->password);
        initProperties();
        ArduinoCloud.begin(*connectionHandler);
        ArduinoCloud.printDebugInfo();
        
        Serial.println("Arduino IoT Cloud initialized");
    }

    void initProperties(){
        ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
        ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
        ArduinoCloud.addProperty(lightSwitch, READWRITE, ON_CHANGE, onLightSwitchChangeStatic);
    }

    void update(){
        if (connectionHandler != nullptr) {
            ArduinoCloud.update();
        }
    }
    
    static void onLightSwitchChangeStatic()
    {
        if (ArduinoServerMgr != nullptr) {
            ArduinoServerMgr->onLightSwitchChange();
        }
    }
    
    void onLightSwitchChange()
    {
        LightBulbMgr->open(lightSwitch);
    }
};

inline ArduinoServerManager *ArduinoServerMgr = new ArduinoServerManager();