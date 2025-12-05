#include "stream_manager.h"
#include "Arduino.h"
#include "camera_manager.h" 

extern CameraManager cameraMgr; 
extern volatile bool captureRequested; 

// Define global streaming state here
volatile bool isStreaming = false;

// Initialize static callback
CaptureCallback StreamManager::captureCb = NULL;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

StreamManager::StreamManager() {
    stream_httpd = NULL;
}

void StreamManager::setCaptureCallback(CaptureCallback cb) {
    captureCb = cb;
}

esp_err_t StreamManager::stream_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    // Auto-init camera if needed
    if (!cameraMgr.isInitialized()) {
        Serial.println("[STREAM] Auto-initializing camera...");
        if (!cameraMgr.init()) {
            return ESP_FAIL;
        }
    }

    isStreaming = true;
    Serial.println("▶️ Stream started");

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("Camera capture failed");
            res = ESP_FAIL;
        } else {
            // Check for capture request
            if (captureRequested) {
                Serial.println("📸 Stream Task: Handling capture request...");
                if (captureCb) {
                    captureCb(fb); // Use callback
                    Serial.println("✅ Stream Task: Capture handled via callback");
                } else {
                    Serial.println("⚠️ Stream Task: No capture callback registered!");
                }
                captureRequested = false; // Reset flag
            }

            if(fb->format != PIXFORMAT_JPEG){
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if(!jpeg_converted){
                    Serial.println("JPEG compression failed");
                    res = ESP_FAIL;
                }
            } else {
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
        // Serial.printf("MJPEG: %uB\n",(uint32_t)(_jpg_buf_len));
    }
    
    isStreaming = false;
    Serial.println("⏹️ Stream stopped");
    return res;
}

void StreamManager::startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 81; // Use port 81 for streaming to avoid conflict if needed, or 80

    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    Serial.printf("Starting web server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
    }
}
