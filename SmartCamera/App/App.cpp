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
#include "Handler/HealthHandler.cpp"


class App {
public:
  httpd_handle_t stream_httpd = NULL;
  httpd_handle_t camera_httpd = NULL;

  void setup();
  void startCameraServer();
};

inline App *app = new App();

void App::setup() {
  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}


void App::startCameraServer() {
  RaFilter::ra_filter_init(&RaFilter::ra_filter, 20);
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 16;

  log_i("Starting web server on port: '%d'", config.server_port);
  if (httpd_start(&camera_httpd, &config) == ESP_OK) {
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
    HealthHandler::registerHandler(camera_httpd);
  }

  config.server_port += 1;
  config.ctrl_port += 1;
  log_i("Starting stream server on port: '%d'", config.server_port);
  if (httpd_start(&stream_httpd, &config) == ESP_OK) {
    StreamHandler::registerHandler(stream_httpd);
  }
}


