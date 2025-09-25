#include "../HttpManager.cpp"

class XclkHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t xclk_uri = {
            .uri = "/xclk",
            .method = HTTP_GET,
            .handler = XclkHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &xclk_uri);
    }
};

esp_err_t XclkHandler::handler(httpd_req_t *req) {
    char *buf = NULL;
    char _xclk[32];

    if (HttpManager::parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }
    if (httpd_query_key_value(buf, "xclk", _xclk, sizeof(_xclk)) != ESP_OK) {
        free(buf);
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }
    free(buf);

    int xclk = atoi(_xclk);
    log_i("Set XCLK: %d MHz", xclk);

    sensor_t *s = esp_camera_sensor_get();
    int res = s->set_xclk(s, LEDC_TIMER_0, xclk);
    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}