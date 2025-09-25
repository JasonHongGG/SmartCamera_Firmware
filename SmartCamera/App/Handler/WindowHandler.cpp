#include "esp_camera.h"
#include "../HttpManager.cpp"

class WindowHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t win_uri = {
            .uri = "/resolution",
            .method = HTTP_GET,
            .handler = WindowHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &win_uri);
    }
};

esp_err_t WindowHandler::handler(httpd_req_t *req) {
  char *buf = NULL;

  if (HttpManager::parse_get(req, &buf) != ESP_OK) {
    return ESP_FAIL;
  }

  int startX = HttpManager::parse_get_var(buf, "sx", 0);
  int startY = HttpManager::parse_get_var(buf, "sy", 0);
  int endX = HttpManager::parse_get_var(buf, "ex", 0);
  int endY = HttpManager::parse_get_var(buf, "ey", 0);
  int offsetX = HttpManager::parse_get_var(buf, "offx", 0);
  int offsetY = HttpManager::parse_get_var(buf, "offy", 0);
  int totalX = HttpManager::parse_get_var(buf, "tx", 0);
  int totalY = HttpManager::parse_get_var(buf, "ty", 0);  // codespell:ignore totaly
  int outputX = HttpManager::parse_get_var(buf, "ox", 0);
  int outputY = HttpManager::parse_get_var(buf, "oy", 0);
  bool scale = HttpManager::parse_get_var(buf, "scale", 0) == 1;
  bool binning = HttpManager::parse_get_var(buf, "binning", 0) == 1;
  free(buf);

  log_i(
    "Set Window: Start: %d %d, End: %d %d, Offset: %d %d, Total: %d %d, Output: %d %d, Scale: %u, Binning: %u", startX, startY, endX, endY, offsetX, offsetY,
    totalX, totalY, outputX, outputY, scale, binning  // codespell:ignore totaly
  );
  sensor_t *s = esp_camera_sensor_get();
  int res = s->set_res_raw(s, startX, startY, endX, endY, offsetX, offsetY, totalX, totalY, outputX, outputY, scale, binning);  // codespell:ignore totaly
  if (res) {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}