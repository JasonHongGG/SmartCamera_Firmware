# pragma once
#include "esp_camera.h"
#include "CameraPins.h"
#include <Arduino.h>

class CameraManager {
public:
    CameraManager();
    ~CameraManager();

    bool start();
    bool stop();

private:
    camera_config_t config;
    bool cameraInitialized = false;
};
inline CameraManager *CameraMgr = new CameraManager();