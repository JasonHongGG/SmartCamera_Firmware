#include "../HttpManager.cpp"
#include "../../FlashLightManager.h"

class CaptureHandler {
public:
    typedef struct {
        httpd_req_t *req;
        size_t len;
    } jpg_chunking_t;

    static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len);

    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t capture_uri = {
            .uri = "/capture",
            .method = HTTP_GET,
            .handler = CaptureHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }
};

size_t CaptureHandler::jpg_encode_stream(void *arg, size_t index, const void *data, size_t len) {
  CaptureHandler::jpg_chunking_t *j = (CaptureHandler::jpg_chunking_t *)arg;
  if (!index) {
    j->len = 0;
  }
  if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK) {
    return 0;
  }
  j->len += len;
  return len;
}

esp_err_t CaptureHandler::handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;

    int64_t fr_start = esp_timer_get_time();

    FlashLightManager::openFlashLight(true);
    vTaskDelay(150 / portTICK_PERIOD_MS);  // The LED needs to be turned on ~150ms before the call to esp_camera_fb_get()
    fb = esp_camera_fb_get();              // or it won't be visible in the frame. A better way to do this is needed.
    FlashLightManager::openFlashLight(false);


    if (!fb) {
        log_e("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char ts[32];
    snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    size_t fb_len = 0;
    if (fb->format == PIXFORMAT_JPEG) {
        fb_len = fb->len;
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    } else {
        CaptureHandler::jpg_chunking_t jchunk = {req, 0};
        res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
        httpd_resp_send_chunk(req, NULL, 0);
        fb_len = jchunk.len;
    }
    esp_camera_fb_return(fb);


    int64_t fr_end = esp_timer_get_time();
    log_i("JPG: %uB %ums", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));
    return res;
}