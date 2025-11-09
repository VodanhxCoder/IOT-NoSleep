/**
 * stream_server.h - HTTP MJPEG Stream Server
 * Provides real-time video streaming capability
 */

#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include <Arduino.h>
#include "esp_camera.h"
#include "esp_http_server.h"

class StreamServer {
private:
    httpd_handle_t stream_httpd = NULL;
    static bool isStreaming;
    
    // MJPEG boundary
    static const char* STREAM_BOUNDARY;
    static const char* STREAM_CONTENT_TYPE;
    
    // Stream handler
    static esp_err_t stream_handler(httpd_req_t *req);
    
public:
    StreamServer();
    bool begin(uint16_t port = 81);
    void stop();
    bool isRunning();
    static int getClientCount();
};

#endif // STREAM_SERVER_H
