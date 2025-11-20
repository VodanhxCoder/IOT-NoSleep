/**
 * upload_manager.h - Image upload to server
 */

#ifndef UPLOAD_MANAGER_H
#define UPLOAD_MANAGER_H

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "config.h"

class UploadManager {
public:
    UploadManager();
    bool upload(camera_fb_t* fb, const String& token);
    bool uploadImage(const uint8_t* buf, size_t len, const String& token);  // Added: direct buffer upload
    int getLastHttpCode();
    String getLastResponse();

private:
    int _lastHttpCode;
    String _lastResponse;
};

#endif // UPLOAD_MANAGER_H
