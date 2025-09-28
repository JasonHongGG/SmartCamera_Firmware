#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "esp32-hal-ledc.h"
#include "sdkconfig.h"
#include "esp32-hal-log.h"


#include "HttpManager.cpp"
#include "../WifiManager.h"

#include "RaFilter.cpp"
#include "Handler/IndexHandler.cpp"
#include "Handler/CmdHandler.cpp"
#include "Handler/StatusHandler.cpp"
#include "Handler/CaptureHandler.cpp"
#include "Handler/BmpHandler.cpp"
#include "Handler/XclkHandler.cpp"
#include "Handler/RegHandler.cpp"
#include "Handler/GregHandler.cpp"
#include "Handler/PllHandler.cpp"
#include "Handler/StreamHandler.cpp"
#include "Handler/WindowHandler.cpp"
#include "Handler/TestConnectionHandler.cpp"
#include "../HttpHealthMonitor.h"


class App {
public:
  httpd_handle_t stream_httpd = NULL;
  httpd_handle_t camera_httpd = NULL;

  void setup();
  void startCameraServer();
  void stopCameraServer();
  void restartCameraServer();
  bool isServerRunning();
  void checkServerHealth();
  
private:
  unsigned long lastServerCheck = 0;
  int serverRestartCount = 0;
  static const unsigned long SERVER_CHECK_INTERVAL = 30000; // 30秒檢查一次
  static const int MAX_RESTART_COUNT = 5; // 最多重啟5次
};

inline App *app = new App();

void App::setup() {
  HttpHealthMonitor::setup();
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}


void App::startCameraServer() {
  RaFilter::ra_filter_init(&RaFilter::ra_filter, 20);
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;
  config.max_open_sockets = 7;        // 限制並發連接數
  config.max_resp_headers = 8;        // 限制響應頭數量
  config.backlog_conn = 5;            // 限制等待隊列
  config.lru_purge_enable = true;     // 啟用 LRU 清理
  config.recv_wait_timeout = 5;       // 接收超時
  config.send_wait_timeout = 5;       // 發送超時

  log_i("Starting web server on port: '%d'", config.server_port);
  esp_err_t ret = httpd_start(&camera_httpd, &config);
  if (ret == ESP_OK) {
    log_i("Web server started successfully");
    IndexHandler::registerHandler(camera_httpd);
    CmdHandler::registerHandler(camera_httpd);
    StatusHandler::registerHandler(camera_httpd);
    CaptureHandler::registerHandler(camera_httpd);
    BmpHandler::registerHandler(camera_httpd);
    XclkHandler::registerHandler(camera_httpd);
    RegHandler::registerHandler(camera_httpd);
    GregHandler::registerHandler(camera_httpd);
    PllHandler::registerHandler(camera_httpd);
    WindowHandler::registerHandler(camera_httpd);
    TestConnectionHandler::registerHandler(camera_httpd);
  } else {
    log_e("Failed to start web server: %s", esp_err_to_name(ret));
  }

  config.server_port += 1;
  config.ctrl_port += 1;
  log_i("Starting stream server on port: '%d'", config.server_port);
  ret = httpd_start(&stream_httpd, &config);
  if (ret == ESP_OK) {
    log_i("Stream server started successfully");
    StreamHandler::registerHandler(stream_httpd);
  } else {
    log_e("Failed to start stream server: %s", esp_err_to_name(ret));
  }
}

void App::stopCameraServer() {
  log_i("Stopping camera servers");
  
  if (camera_httpd) {
    httpd_stop(camera_httpd);
    camera_httpd = NULL;
    log_i("Web server stopped");
  }
  
  if (stream_httpd) {
    httpd_stop(stream_httpd);
    stream_httpd = NULL;
    log_i("Stream server stopped");
  }
}

void App::restartCameraServer() {
  log_i("Restarting camera servers (attempt %d/%d)", serverRestartCount + 1, MAX_RESTART_COUNT);
  
  stopCameraServer();
  delay(2000); // 等待完全停止
  
  // 清理可能的記憶體碎片
  if (heap_caps_check_integrity_all(true)) {
    log_i("Heap integrity check passed");
  } else {
    log_w("Heap integrity issues detected");
  }
  
  startCameraServer();
  serverRestartCount++;
}

bool App::isServerRunning() {
  // 檢查服務器是否仍在運行
  return (camera_httpd != NULL && stream_httpd != NULL);
}

void App::checkServerHealth() {
  unsigned long currentTime = millis();
  
  // 每30秒檢查一次
  if (currentTime - lastServerCheck < SERVER_CHECK_INTERVAL) {
    return;
  }
  
  lastServerCheck = currentTime;
  
  // 檢查 HTTP 健康監控是否建議重啟
  if (HttpHealthMonitor::shouldRestartServer()) {
    log_w("HTTP Health Monitor suggests server restart");
    if (serverRestartCount < MAX_RESTART_COUNT) {
      HttpHealthMonitor::resetErrorCount();
      restartCameraServer();
    }
    return;
  }
  
  if (!isServerRunning()) {
    log_w("Server health check failed - servers not running");
    
    if (serverRestartCount < MAX_RESTART_COUNT) {
      restartCameraServer();
    } else {
      log_e("Max restart attempts reached. System needs manual intervention.");
      // 可以考慮設置一個標誌，通過 watchdog 重啟整個系統
    }
    return;
  }
  
  // 檢查記憶體使用情況
  uint32_t freeHeap = ESP.getFreeHeap();
  if (freeHeap < 20000) { // 少於20KB時警告
    log_w("Low memory detected: %u bytes free", freeHeap);
    
    if (freeHeap < 10000) { // 少於10KB時重啟服務器
      log_e("Critical memory shortage, restarting servers");
      if (serverRestartCount < MAX_RESTART_COUNT) {
        restartCameraServer();
      }
    }
  }
  
  // 重置重啟計數器（如果系統已經穩定運行一段時間）
  if (serverRestartCount > 0 && currentTime > 600000) { // 10分鐘後重置計數器
    static unsigned long lastReset = 0;
    if (currentTime - lastReset > 600000) {
      serverRestartCount = 0;
      lastReset = currentTime;
      log_i("Server restart counter reset after stable operation");
    }
  }
  
  log_d("Server health check passed - Free heap: %u bytes", freeHeap);
}


