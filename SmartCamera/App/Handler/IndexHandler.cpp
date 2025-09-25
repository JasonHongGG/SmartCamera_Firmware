#include "../HttpManager.cpp"
#include "../WebIndex.h"

class IndexHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = IndexHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &index_uri);
    }
};

esp_err_t IndexHandler::handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
    sensor_t *s = esp_camera_sensor_get();
    if (s != NULL) {
        return httpd_resp_send(req, (const char *)index_ov2640_html_gz, index_ov2640_html_gz_len);
    } else {
        log_e("Camera sensor not found");
        return httpd_resp_send_500(req);
    }
}