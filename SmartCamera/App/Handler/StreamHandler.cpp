#include "../HttpManager.cpp"
#include "../../FlashLightManager.h"
#include "../../CameraManager.h"
#define PART_BOUNDARY "123456789000000000000987654321"

class StreamHandler {
public:
    static const char *_STREAM_CONTENT_TYPE;
    static const char *_STREAM_BOUNDARY;
    static const char *_STREAM_PART;

    static esp_err_t handler(httpd_req_t *req);

    static void registerHandler(httpd_handle_t camera_httpd) {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = StreamHandler::handler,
            .user_ctx = NULL
        };

        httpd_register_uri_handler(camera_httpd, &stream_uri);
    }
};

const char *StreamHandler::_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
const char *StreamHandler::_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
const char *StreamHandler::_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

esp_err_t StreamHandler::handler(httpd_req_t *req) {
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    static int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, StreamHandler::_STREAM_CONTENT_TYPE);
    if (res != ESP_OK) {
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    FlashLightManager::isStreaming = true;
    FlashLightManager::openFlashLight(true);

    while (true) {
        // 檢查相機是否仍然可用
        if (!CameraMgr->isInitialized()) {
            log_i("Camera not initialized, stopping stream");
            res = ESP_FAIL;
            break;
        }
        
        fb = esp_camera_fb_get();
        if (!fb) {
            log_e("Camera capture failed");
            res = ESP_FAIL;
            break; // 退出循環而不是繼續嘗試
        } else {
        _timestamp.tv_sec = fb->timestamp.tv_sec;
        _timestamp.tv_usec = fb->timestamp.tv_usec;
        if (fb->format != PIXFORMAT_JPEG) {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            esp_camera_fb_return(fb);
            fb = NULL;
            if (!jpeg_converted) {
            log_e("JPEG compression failed");
            res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
        }
        if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, StreamHandler::_STREAM_BOUNDARY, strlen(StreamHandler::_STREAM_BOUNDARY));
        }
        if (res == ESP_OK) {
        size_t hlen = snprintf((char *)part_buf, 128, StreamHandler::_STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if (fb) {
        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;
        } else if (_jpg_buf) {
        free(_jpg_buf);
        _jpg_buf = NULL;
        }
        if (res != ESP_OK) {
        log_e("Send frame failed");
        break;
        }
        int64_t fr_end = esp_timer_get_time();

        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;

        frame_time /= 1000;

        uint32_t avg_frame_time = RaFilter::ra_filter_run(&RaFilter::ra_filter, frame_time);
        log_i(
        "MJPG: %uB %ums (%.1ffps), AVG: %ums (%.1ffps)", (uint32_t)(_jpg_buf_len), (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time, avg_frame_time,
        1000.0 / avg_frame_time
        );
    }

    FlashLightManager::isStreaming = false;
    FlashLightManager::openFlashLight(false);

    return res;
}