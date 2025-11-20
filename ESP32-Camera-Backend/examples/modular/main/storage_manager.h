/**
 * storage_manager.h
 * Handles SD card mounting plus pending/sent image queues.
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <SD_MMC.h>
#include <time.h>
#include "esp_camera.h"
#include "upload_manager.h"

typedef void (*PendingUploadCallback)(size_t index, const String& path);

struct PendingSummary {
    size_t count = 0;
    time_t oldestTimestamp = 0;
    time_t latestTimestamp = 0;
};

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
     * @return true if there are any files waiting in /pending.
     */
    bool hasPending();

    /**
     * Fill PendingSummary with queue stats. Returns false if queue empty.
     */
    bool getPendingSummary(PendingSummary& summary);

    /**
     * Iterate files in /pending, attempt upload, move to /sent on success.
     * Returns number of files uploaded during this pass.
     */
    size_t flushPendingQueue(const String& token,
                             UploadManager& uploader,
                             size_t maxFiles = SIZE_MAX,
                             PendingUploadCallback onFileStart = nullptr);

private:
    bool _sdReady;

    bool ensureDirectories();
    String buildPendingPath() const;
    bool moveToSent(const String& pendingPath);
    time_t timestampFromFilename(const String& path) const;
};

#endif // STORAGE_MANAGER_H
