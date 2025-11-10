/**
 * storage_manager.h
 * Handles SD card mounting plus pending/sent image queues.
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include "esp_camera.h"
#include "upload_manager.h"

class StorageManager {
public:
    StorageManager();

    /**
     * Mount the SD card and make sure pending/sent folders exist.
     * Returns true when the queues can be used this boot.
     */
    bool begin();

    /**
     * @return true when the SD queue can be used.
     */
    bool isReady() const;

    /**
     * Persist the provided framebuffer into /pending with a timestamped name.
     * Used when uploads fail so the image can be retried later.
     */
    bool savePendingFrame(const camera_fb_t* fb);

    /**
     * Iterate every file in /pending, attempt an HTTP upload, then move the
     * file to /sent (or leave it in /pending on failure).
     */
    void flushPendingQueue(const String& token, UploadManager& uploader);

private:
    bool _sdReady;

    bool ensureDirectories();
    String buildPendingPath() const;
    bool moveToSent(const String& pendingPath);
};

#endif // STORAGE_MANAGER_H
