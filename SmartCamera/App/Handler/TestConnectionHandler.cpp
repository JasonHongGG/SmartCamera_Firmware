#pragma once
#include "../HttpManager.cpp"

class TestConnectionHandler {
public:
    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t test_connection_uri = {
            .uri = "/testConnection",
            .method = HTTP_GET,
            .handler = TestConnectionHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &test_connection_uri);
    }
};

esp_err_t TestConnectionHandler::handler(httpd_req_t *req) {
    static char json_response[256];
    
    // 獲取當前時間戳
    unsigned long currentTime = millis();
    
    // 獲取系統信息
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    
    // 準備響應 JSON
    snprintf(json_response, sizeof(json_response),
             "{"
             "\"status\":\"connected\","
             "\"message\":\"ESP32-CAM connection OK\","
             "\"timestamp\":%lu,"
             "\"uptime\":%lu,"
             "\"free_heap\":%u,"
             "\"total_heap\":%u,"
             "\"wifi_rssi\":%d"
             "}",
             currentTime,
             currentTime, // uptime 也是 millis()
             freeHeap,
             totalHeap,
             WiFi.RSSI()
    );

    // 設置響應頭
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    
    // 發送響應
    return httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);
}