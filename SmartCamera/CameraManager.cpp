#include "CameraManager.h"

CameraManager::CameraManager() {
    this->config.ledc_channel = LEDC_CHANNEL_0;
    this->config.ledc_timer = LEDC_TIMER_0;
    this->config.pin_d0 = Y2_GPIO_NUM;
    this->config.pin_d1 = Y3_GPIO_NUM;
    this->config.pin_d2 = Y4_GPIO_NUM;
    this->config.pin_d3 = Y5_GPIO_NUM;
    this->config.pin_d4 = Y6_GPIO_NUM;
    this->config.pin_d5 = Y7_GPIO_NUM;
    this->config.pin_d6 = Y8_GPIO_NUM;
    this->config.pin_d7 = Y9_GPIO_NUM;
    this->config.pin_xclk = XCLK_GPIO_NUM;
    this->config.pin_pclk = PCLK_GPIO_NUM;
    this->config.pin_vsync = VSYNC_GPIO_NUM;
    this->config.pin_href = HREF_GPIO_NUM;
    this->config.pin_sccb_sda = SIOD_GPIO_NUM;
    this->config.pin_sccb_scl = SIOC_GPIO_NUM;
    this->config.pin_pwdn = PWDN_GPIO_NUM;
    this->config.pin_reset = RESET_GPIO_NUM;
    this->config.xclk_freq_hz = 20000000;
    this->config.frame_size = FRAMESIZE_UXGA;
    this->config.pixel_format = PIXFORMAT_JPEG;  // for streaming
    //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
    this->config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    this->config.fb_location = CAMERA_FB_IN_PSRAM;
    this->config.jpeg_quality = 12;
    this->config.fb_count = 1;

    // if PSRAM IC present, init with UXGA resolution and higher JPEG quality for larger pre-allocated frame buffer.
    if (this->config.pixel_format == PIXFORMAT_JPEG) {
        if (psramFound()) {
        this->config.jpeg_quality = 10;
            this->config.fb_count = 2;
            this->config.grab_mode = CAMERA_GRAB_LATEST;
        } else {
        // Limit the frame size when PSRAM is not available
        this->config.frame_size = FRAMESIZE_SVGA;
        this->config.fb_location = CAMERA_FB_IN_DRAM;
        }
    } else {
        // Best option for face detection/recognition
        this->config.frame_size = FRAMESIZE_240X240;
    }
}


bool CameraManager::start() {
    Serial.println("CameraManager::start");
    if (this->cameraInitialized) {
        Serial.println("Camera already initialized");
        return true;
    }

    pinMode(this->config.pin_reset, OUTPUT);
    digitalWrite(this->config.pin_reset, LOW);
    delay(10);
    digitalWrite(this->config.pin_reset, HIGH);
    delay(10);

    // camera init
    esp_err_t err = esp_camera_init(&this->config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    // drop down frame size for higher initial frame rate
    if (this->config.pixel_format == PIXFORMAT_JPEG) {
        s->set_framesize(s, FRAMESIZE_QVGA);
    }

    this->cameraInitialized = true;
    return true;
}

bool CameraManager::stop() {
    Serial.println("CameraManager::stop");
    if (!this->cameraInitialized) return false;
    esp_camera_deinit();
    this->cameraInitialized = false;
    return true;
}
