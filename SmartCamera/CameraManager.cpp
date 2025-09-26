#include "CameraManager.h"
#include <Wire.h>

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
    this->config.xclk_freq_hz = 10000000; // 降低時鐘頻率以提高穩定性
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
        // 執行健康檢查
        if (!healthCheck()) {
            Serial.println("Camera health check failed, restarting...");
            return restart();
        }
        return true;
    }

    return initCamera();
}

bool CameraManager::initCamera() {
    Serial.println("CameraManager::initCamera");
    
    // 檢查記憶體是否足夠
    uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 50000) { // 需要至少50KB空閒記憶體
        Serial.printf("Insufficient memory for camera init: %u bytes\n", freeHeap);
        return false;
    }
    
    // 執行硬體重置
    resetCamera();
    
    // 嘗試初始化相機，最多重試 MAX_RETRY_COUNT 次
    for (int retry = 0; retry < MAX_RETRY_COUNT; retry++) {
        Serial.printf("Camera init attempt %d/%d (Free heap: %u)\n", 
                     retry + 1, MAX_RETRY_COUNT, ESP.getFreeHeap());
        
        // 每次重試前進行電源循環和更長的等待
        if (retry > 0) {
            powerCycle();
            delay(1000); // 增加等待時間
        }
        
        // 在初始化前先清理可能的殘留狀態
        esp_camera_deinit(); // 確保完全清理
        delay(100);
        
        esp_err_t err = esp_camera_init(&this->config);
        if (err == ESP_OK) {
            Serial.println("Camera init successful");
            
            // 等待感應器穩定
            delay(200);
            
            // 驗證感應器是否可用
            sensor_t *s = esp_camera_sensor_get();
            if (s != NULL) {
                // 嘗試讀取感應器 ID 來驗證通信
                uint16_t sensor_id = s->id.PID;
                Serial.printf("Camera sensor ID: 0x%x\n", sensor_id);
                
                if (sensor_id != 0 && sensor_id != 0xFFFF) {
                    // 設置初始幀大小
                    if (this->config.pixel_format == PIXFORMAT_JPEG) {
                        s->set_framesize(s, FRAMESIZE_QVGA);
                    }
                    
                    // 進行一次測試拍照以確保一切正常
                    camera_fb_t *fb = esp_camera_fb_get();
                    if (fb != NULL) {
                        esp_camera_fb_return(fb);
                        Serial.println("Camera test capture successful");
                        
                        this->cameraInitialized = true;
                        this->initRetryCount = 0;
                        return true;
                    } else {
                        Serial.println("Camera test capture failed");
                    }
                } else {
                    Serial.printf("Invalid sensor ID: 0x%x\n", sensor_id);
                }
                
                esp_camera_deinit();
            } else {
                Serial.println("Camera sensor not available");
                esp_camera_deinit();
            }
        } else {
            Serial.printf("Camera init failed with error 0x%x (%s)\n", err, esp_err_to_name(err));
            
            // 特別處理 I2C 相關錯誤
            if (err == ESP_ERR_NOT_FOUND || err == ESP_ERR_TIMEOUT) {
                Serial.println("I2C communication error detected, performing extended reset...");
                extendedReset();
            }
        }
        
        delay(1000); // 增加重試間隔
    }
    
    Serial.printf("Camera init failed after %d attempts\n", MAX_RETRY_COUNT);
    this->initRetryCount++;
    return false;
}

bool CameraManager::stop() {
    Serial.println("CameraManager::stop");
    if (!this->cameraInitialized) {
        Serial.println("Camera not initialized");
        return true;
    }
    
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        Serial.printf("Camera deinit failed with error 0x%x (%s)\n", err, esp_err_to_name(err));
    } else {
        Serial.println("Camera deinit successful");
    }
    
    this->cameraInitialized = false;
    return true;
}

bool CameraManager::restart() {
    Serial.println("CameraManager::restart");
    stop();
    delay(100); // 等待完全停止
    return start();
}

bool CameraManager::healthCheck() {
    Serial.println("CameraManager::healthCheck");
    if (!this->cameraInitialized) {
        return false;
    }
    
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        Serial.println("Health check failed: sensor not available");
        return false;
    }
    
    // 嘗試獲取一個測試幀來驗證相機是否正常工作
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb == NULL) {
        Serial.println("Health check failed: cannot capture frame");
        return false;
    }
    
    esp_camera_fb_return(fb);
    Serial.println("Health check passed");
    return true;
}

void CameraManager::resetCamera() {
    Serial.println("CameraManager::resetCamera");
    
    // 硬體重置相機模組
    if (this->config.pin_reset != -1) {
        pinMode(this->config.pin_reset, OUTPUT);
        digitalWrite(this->config.pin_reset, LOW);
        delay(50); // 延長重置時間
        digitalWrite(this->config.pin_reset, HIGH);
        delay(50);
    }
    
    // 電源管理 pin 控制
    if (this->config.pin_pwdn != -1) {
        pinMode(this->config.pin_pwdn, OUTPUT);
        digitalWrite(this->config.pin_pwdn, HIGH); // 關閉電源
        delay(50);
        digitalWrite(this->config.pin_pwdn, LOW);  // 開啟電源
        delay(50);
    }
}

void CameraManager::powerCycle() {
    Serial.println("CameraManager::powerCycle");
    
    // 電源循環：關閉電源 -> 等待 -> 開啟電源
    if (this->config.pin_pwdn != -1) {
        digitalWrite(this->config.pin_pwdn, HIGH); // 關閉電源
        delay(200);
        digitalWrite(this->config.pin_pwdn, LOW);  // 開啟電源
        delay(200);
    }
    
    // 額外的 I2C 匯流排重置
    // 清除可能的 I2C 狀態
    Wire.begin(this->config.pin_sccb_sda, this->config.pin_sccb_scl);
    Wire.end();
    delay(100);
}

void CameraManager::extendedReset() {
    Serial.println("CameraManager::extendedReset - Performing comprehensive reset");
    
    // 1. 完全斷電
    if (this->config.pin_pwdn != -1) {
        digitalWrite(this->config.pin_pwdn, HIGH);
        delay(500); // 延長斷電時間
    }
    
    // 2. 硬體重置
    if (this->config.pin_reset != -1) {
        digitalWrite(this->config.pin_reset, LOW);
        delay(100);
        digitalWrite(this->config.pin_reset, HIGH);
        delay(100);
    }
    
    // 3. I2C 匯流排完全重置
    Wire.begin(this->config.pin_sccb_sda, this->config.pin_sccb_scl);
    
    // 發送 I2C 停止條件來清除匯流排
    for (int i = 0; i < 10; i++) {
        Wire.beginTransmission(0x00);
        Wire.endTransmission();
        delay(10);
    }
    
    Wire.end();
    delay(200);
    
    // 4. 重新上電
    if (this->config.pin_pwdn != -1) {
        digitalWrite(this->config.pin_pwdn, LOW);
        delay(300); // 等待電源穩定
    }
    
    Serial.println("Extended reset completed");
}
