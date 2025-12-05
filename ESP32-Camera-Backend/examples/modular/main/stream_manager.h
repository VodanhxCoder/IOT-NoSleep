#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include "esp_camera.h"
#include "esp_http_server.h"

// Define callback type for capturing frames
typedef void (*CaptureCallback)(camera_fb_t*);

class StreamManager {
public:
    StreamManager();
    void startWebServer();
    void handleClient(); 
    
    // Register callback for processing captured frames
    static void setCaptureCallback(CaptureCallback cb);

private:
    httpd_handle_t stream_httpd = NULL;
    static esp_err_t stream_handler(httpd_req_t *req);
    static CaptureCallback captureCb; // Static member to be accessible from static handler
};

#endif // STREAM_MANAGER_H
