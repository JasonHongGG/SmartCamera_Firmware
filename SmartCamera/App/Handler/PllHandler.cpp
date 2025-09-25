#include "../HttpManager.cpp"

class PllHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t pll_uri = {
            .uri = "/pll",
            .method = HTTP_GET,
            .handler = PllHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &pll_uri);
    }
};

esp_err_t PllHandler::handler(httpd_req_t *req) {
    char *buf = NULL;

    if (HttpManager::parse_get(req, &buf) != ESP_OK) {
        return ESP_FAIL;
    }

    int bypass = HttpManager::parse_get_var(buf, "bypass", 0);
    int mul = HttpManager::parse_get_var(buf, "mul", 0);
    int sys = HttpManager::parse_get_var(buf, "sys", 0);
    int root = HttpManager::parse_get_var(buf, "root", 0);
    int pre = HttpManager::parse_get_var(buf, "pre", 0);
    int seld5 = HttpManager::parse_get_var(buf, "seld5", 0);
    int pclken = HttpManager::parse_get_var(buf, "pclken", 0);
    int pclk = HttpManager::parse_get_var(buf, "pclk", 0);
    free(buf);

    log_i("Set Pll: bypass: %d, mul: %d, sys: %d, root: %d, pre: %d, seld5: %d, pclken: %d, pclk: %d", bypass, mul, sys, root, pre, seld5, pclken, pclk);
    sensor_t *s = esp_camera_sensor_get();
    int res = s->set_pll(s, bypass, mul, sys, root, pre, seld5, pclken, pclk);
    if (res) {
        return httpd_resp_send_500(req);
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, NULL, 0);
}