# pragma once
#include "esp_camera.h"
#include "CameraPins.h"
#include <Arduino.h>

class CameraManager {
public:
    bool camera_open = true;

public:
    CameraManager();
    ~CameraManager();

    bool start();
    bool stop();
    bool restart();
    bool isInitialized() { return cameraInitialized; }
    bool healthCheck();
    void resetCamera();

private:
    camera_config_t config;
    bool cameraInitialized = false;
    int initRetryCount = 0;
    static const int MAX_RETRY_COUNT = 3;
    
    bool initCamera();
    void powerCycle();
    void extendedReset();
};
inline CameraManager *CameraMgr = new CameraManager();