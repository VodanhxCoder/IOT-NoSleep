/**
 * stream_server.cpp - HTTP MJPEG Stream Server Implementation
 */

#include "stream_server.h"
#include "config.h"

// Static members
bool StreamServer::isStreaming = false;
const char* StreamServer::STREAM_BOUNDARY = "123456789000000000000987654321";
const char* StreamServer::STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" "123456789000000000000987654321";

// Stream handler - sends MJPEG frames continuously
esp_err_t StreamServer::stream_handler(httpd_req_t *req) {
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char * part_buf[64];

    Serial.println("📹 Stream client connected");

    // Set response headers for MJPEG stream
    res = httpd_resp_set_type(req, STREAM_CONTENT_TYPE);
    if(res != ESP_OK) {
        Serial.println("❌ Failed to set stream content type");
        return res;
    }

    isStreaming = true;
    Serial.println("🎬 Starting stream loop...");
    int frameCount = 0;
    
    while(true) {
        fb = esp_camera_fb_get();
        if (!fb) {
            Serial.println("⚠️ Camera capture failed during stream");
            res = ESP_FAIL;
            break;
        }

        if(fb->format != PIXFORMAT_JPEG) {
            // If not JPEG, we need to convert (shouldn't happen with our config)
            Serial.println("⚠️ Non-JPEG frame, skipping");
            esp_camera_fb_return(fb);
            continue;
        }

        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
        
        frameCount++;
        if (frameCount % 30 == 1) { // Log every 30 frames
            Serial.printf("📹 Streaming frame #%d (%d bytes)\n", frameCount, _jpg_buf_len);
        }

        // Send JPEG as part of multipart stream
        size_t hlen = snprintf((char *)part_buf, 64, 
            "\r\n--" "123456789000000000000987654321" "\r\n"
            "Content-Type: image/jpeg\r\n"
            "Content-Length: %u\r\n\r\n", 
            _jpg_buf_len);
        
        res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        
        if(res == ESP_OK) {
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }

        esp_camera_fb_return(fb);
        fb = NULL;
        _jpg_buf = NULL;

        if(res != ESP_OK) {
            Serial.println("📴 Stream client disconnected");
            break;
        }

        // Control frame rate (adjust delay for FPS)
        // 15 FPS = ~66ms delay, 10 FPS = 100ms
        #ifdef STREAM_FPS
            delay(1000 / STREAM_FPS);
        #else
            delay(66); // Default ~15 FPS
        #endif
    }

    isStreaming = false;
    Serial.println("🔚 Stream ended");
    return res;
}

StreamServer::StreamServer() {
    // Constructor
}

bool StreamServer::begin(uint16_t port) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.ctrl_port = port + 1;
    config.max_open_sockets = 3; // Limit concurrent connections
    config.lru_purge_enable = true;

    // Configure stream URI handler
    httpd_uri_t stream_uri = {
        .uri       = "/stream",
        .method    = HTTP_GET,
        .handler   = stream_handler,
        .user_ctx  = NULL
    };

    Serial.printf("🎥 Starting stream server on port %d...\n", port);
    
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        Serial.println("✅ Stream server started!");
        Serial.printf("   Access stream at: http://ESP32_IP:%d/stream\n", port);
        return true;
    }
    
    Serial.println("❌ Failed to start stream server");
    return false;
}

void StreamServer::stop() {
    if (stream_httpd) {
        httpd_stop(stream_httpd);
        stream_httpd = NULL;
        isStreaming = false;
        Serial.println("🛑 Stream server stopped");
    }
}

bool StreamServer::isRunning() {
    return stream_httpd != NULL;
}

int StreamServer::getClientCount() {
    return isStreaming ? 1 : 0; // Simplified - could track actual clients
}
