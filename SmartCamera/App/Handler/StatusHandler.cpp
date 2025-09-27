#pragma once
#include "../HttpManager.cpp"
#include "../../CameraManager.h"
#include "../../LightBulbManager.h"

class StatusHandler {
public:
    static int print_reg(char *p, sensor_t *s, uint16_t reg, uint32_t mask);

    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t status_uri = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = StatusHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &status_uri);
    }
};

int StatusHandler::print_reg(char *p, sensor_t *s, uint16_t reg, uint32_t mask) {
  return sprintf(p, "\"0x%x\":%u,", reg, s->get_reg(s, reg, mask));
}

esp_err_t StatusHandler::handler(httpd_req_t *req) {
    static char json_response[1024];

    sensor_t *s = esp_camera_sensor_get();
    char *p = json_response;
    *p++ = '{';

    p += sprintf(p, "\"camera_open\":%u,", CameraMgr->camera_open);
    p += sprintf(p, "\"light_bulb\":%u,", LightBulbMgr->light_bulb);
    p += sprintf(p, "\"led_intensity\":%u,", FlashLightManager::led_duty);

    if (s == NULL) {
        if (s->id.PID == OV2640_PID) {
            p += print_reg(p, s, 0xd3, 0xFF);
            p += print_reg(p, s, 0x111, 0xFF);
            p += print_reg(p, s, 0x132, 0xFF);
        }

        p += sprintf(p, "\"xclk\":%u,", s->xclk_freq_hz / 1000000);
        p += sprintf(p, "\"pixformat\":%u,", s->pixformat);
        p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
        p += sprintf(p, "\"quality\":%u,", s->status.quality);
        p += sprintf(p, "\"brightness\":%d,", s->status.brightness);
        p += sprintf(p, "\"contrast\":%d,", s->status.contrast);
        p += sprintf(p, "\"saturation\":%d,", s->status.saturation);
        p += sprintf(p, "\"sharpness\":%d,", s->status.sharpness);
        p += sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
        p += sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
        p += sprintf(p, "\"awb\":%u,", s->status.awb);
        p += sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
        p += sprintf(p, "\"aec\":%u,", s->status.aec);
        p += sprintf(p, "\"aec2\":%u,", s->status.aec2);
        p += sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
        p += sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
        p += sprintf(p, "\"agc\":%u,", s->status.agc);
        p += sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
        p += sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
        p += sprintf(p, "\"bpc\":%u,", s->status.bpc);
        p += sprintf(p, "\"wpc\":%u,", s->status.wpc);
        p += sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
        p += sprintf(p, "\"lenc\":%u,", s->status.lenc);
        p += sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
        p += sprintf(p, "\"dcw\":%u,", s->status.dcw);
        p += sprintf(p, "\"colorbar\":%u,", s->status.colorbar);
    }
    
    p += sprintf(p, "\"end\":%u", true);
    *p++ = '}';
    *p++ = 0;
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}