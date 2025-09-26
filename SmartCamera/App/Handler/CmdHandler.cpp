#include <Arduino.h>
#include "../HttpManager.cpp"
#include "../../FlashLightManager.h"
#include "../../CameraManager.h"
#include "../../LightBulbManager.h"

#include "StatusHandler.cpp"

class CmdHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t cmd_uri = {
            .uri = "/control",
            .method = HTTP_GET,
            .handler = CmdHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &cmd_uri);
    }
};

esp_err_t CmdHandler::handler(httpd_req_t *req) {
    char *buf = NULL;
    char variable[32];
    char value[32];

    if (HttpManager::parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK || httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    // 處理布林值和數字值
    int val = 0;
    if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0) {
        val = 1;
    } else if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0) {
        val = 0;
    } else {
        // 嘗試解析為數字
        val = atoi(value);
    }
    
    log_i("%s = %d", variable, val);
    sensor_t *s = esp_camera_sensor_get();
    int res = 0;

    printf("variable: %s | value: %d (original: %s)\n", variable, val, value);
    if (!strcmp(variable, "framesize")) {
        if (s->pixformat == PIXFORMAT_JPEG) {
        res = s->set_framesize(s, (framesize_t)val);
        }
    } else if (!strcmp(variable, "quality")) {
        res = s->set_quality(s, val);
    } else if (!strcmp(variable, "contrast")) {
        res = s->set_contrast(s, val);
    } else if (!strcmp(variable, "brightness")) {
        res = s->set_brightness(s, val);
    } else if (!strcmp(variable, "saturation")) {
        res = s->set_saturation(s, val);
    } else if (!strcmp(variable, "gainceiling")) {
        res = s->set_gainceiling(s, (gainceiling_t)val);
    } else if (!strcmp(variable, "colorbar")) {
        res = s->set_colorbar(s, val);
    } else if (!strcmp(variable, "camera_open")) {
        bool success = false;
        if(val) {
            Serial.println("Attempting to start camera...");
            success = CameraMgr->start();
            if (!success) {
                Serial.println("Camera start failed, attempting restart...");
                success = CameraMgr->restart();
            }
        } else {
            Serial.println("Attempting to stop camera...");
            success = CameraMgr->stop();
        }
        
        // 只有在操作成功時才更新狀態
        if (success) {
            StatusHandler::camera_open = val;
            res = 1;
        } else {
            Serial.printf("Camera operation failed (open=%d)\n", val);
            res = 0; // 操作失敗
        }
    }
    else if (!strcmp(variable, "light_bulb")) {
        StatusHandler::light_bulb = val;
        if(val)
            LightBulbMgr->open(true);
        else
            LightBulbMgr->open(false);
        res = 1;
    }
    else if (!strcmp(variable, "awb")) {
        res = s->set_whitebal(s, val);
    } else if (!strcmp(variable, "agc")) {
        res = s->set_gain_ctrl(s, val);
    } else if (!strcmp(variable, "aec")) {
        res = s->set_exposure_ctrl(s, val);
    } else if (!strcmp(variable, "hmirror")) {
        res = s->set_hmirror(s, val);
    } else if (!strcmp(variable, "vflip")) {
        res = s->set_vflip(s, val);
    } else if (!strcmp(variable, "awb_gain")) {
        res = s->set_awb_gain(s, val);
    } else if (!strcmp(variable, "agc_gain")) {
        res = s->set_agc_gain(s, val);
    } else if (!strcmp(variable, "aec_value")) {
        res = s->set_aec_value(s, val);
    } else if (!strcmp(variable, "aec2")) {
        res = s->set_aec2(s, val);
    } else if (!strcmp(variable, "dcw")) {
        res = s->set_dcw(s, val);
    } else if (!strcmp(variable, "bpc")) {
        res = s->set_bpc(s, val);
    } else if (!strcmp(variable, "wpc")) {
        res = s->set_wpc(s, val);
    } else if (!strcmp(variable, "raw_gma")) {
        res = s->set_raw_gma(s, val);
    } else if (!strcmp(variable, "lenc")) {
        res = s->set_lenc(s, val);
    } else if (!strcmp(variable, "special_effect")) {
        res = s->set_special_effect(s, val);
    } else if (!strcmp(variable, "wb_mode")) {
        res = s->set_wb_mode(s, val);
    } else if (!strcmp(variable, "ae_level")) {
        res = s->set_ae_level(s, val);
    }
    else if (!strcmp(variable, "led_intensity")) {
        FlashLightManager::led_duty = val;
        if (FlashLightManager::isStreaming) {
            FlashLightManager::openFlashLight(true);
        }
    }
    else {
        log_i("Unknown command: %s", variable);
        res = -1;
    }

    if (res < 0) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}