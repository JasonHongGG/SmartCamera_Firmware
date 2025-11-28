# pragma once
#include "esp_camera.h"
#include "CameraPins.h"
#include <Arduino.h>

class CameraManager {
public:
    bool camera_open = true;
    // 當解析度或其他關鍵設定變更時，用來通知串流 handler 儘快結束目前連線
    volatile bool stopStreamRequested = false;

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