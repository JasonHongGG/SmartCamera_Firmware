# pragma once
#include "WifiManager.h"
#include "LightBulbManager.h"
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

class ArduinoServerManager {
public:
    CloudSwitch lightSwitch;
    static constexpr const char* DEVICE_KEY = "xYv9#1xW13lrSMjrrVVjIl90K";
    static constexpr const char* DEVICE_LOGIN_NAME = "4e2a1e1b-e0c8-40b3-a902-53f1dae42818";
public:

    ArduinoServerManager() {
        WiFiConnectionHandler ArduinoIoTPreferredConnection(WiFiMgr->ssid, WiFiMgr->password);
        initProperties();
        ArduinoCloud.begin(ArduinoIoTPreferredConnection);
        ArduinoCloud.printDebugInfo();
    }

    void initProperties(){

        ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
        ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
        ArduinoCloud.addProperty(lightSwitch, READWRITE, ON_CHANGE, onLightSwitchChangeStatic);
    }

    void update(){
        ArduinoCloud.update();
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