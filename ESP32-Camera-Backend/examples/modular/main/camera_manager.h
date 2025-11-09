/**
 * camera_manager.h - Camera initialization and capture
 */

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "esp_camera.h"
#include "config.h"

class CameraManager {
public:
    CameraManager();
    bool init();
    void deinit();
    camera_fb_t* capture();
    void returnFrameBuffer(camera_fb_t* fb);
    bool isInitialized();

private:
    bool _initialized;
    void configureSensor();
};

#endif // CAMERA_MANAGER_H
