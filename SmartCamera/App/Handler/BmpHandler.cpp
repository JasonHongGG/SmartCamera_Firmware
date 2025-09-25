#include "../HttpManager.cpp"

class BmpHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t bmp_uri = {
            .uri = "/bmp",
            .method = HTTP_GET,
            .handler = BmpHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &bmp_uri);
    }
};

esp_err_t BmpHandler::handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;

    uint64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        log_e("Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/x-windows-bmp");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.bmp");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    char ts[32];
    snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    uint8_t *buf = NULL;
    size_t buf_len = 0;
    bool converted = frame2bmp(fb, &buf, &buf_len);
    esp_camera_fb_return(fb);
    if (!converted) {
        log_e("BMP Conversion failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    res = httpd_resp_send(req, (const char *)buf, buf_len);
    free(buf);

    uint64_t fr_end = esp_timer_get_time();
    log_i("BMP: %llums, %uB", (uint64_t)((fr_end - fr_start) / 1000), buf_len);
    return res;
}