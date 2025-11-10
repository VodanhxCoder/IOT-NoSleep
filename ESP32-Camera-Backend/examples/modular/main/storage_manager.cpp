/**
 * storage_manager.cpp
 * Implements SD-based offline queue for captured images.
 */

#include <time.h>
#include <stdlib.h>
#include <esp32-hal-psram.h>
#include "storage_manager.h"

// Base folders to keep pending and sent images separate.
static const char* BASE_DIR = "/esp32cam";
static const char* PENDING_DIR = "/esp32cam/pending";
static const char* SENT_DIR = "/esp32cam/sent";

StorageManager::StorageManager() : _sdReady(false) {}

bool StorageManager::begin() {
    Serial.println("\n[0/6] Mounting SD card...");

#ifdef SD_MMC_CLK
    Serial.printf("[SD] setPins CLK=%d CMD=%d D0=%d\n", (int)SD_MMC_CLK, (int)SD_MMC_CMD, (int)SD_MMC_D0);
    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
#endif

    bool mounted = SD_MMC.begin("/sdcard", false, false);
    if (!mounted) {
        Serial.println("[WARN] SD 4-bit mode failed, retrying 1-bit...");
        mounted = SD_MMC.begin("/sdcard", true, false);
    }

    if (mounted) {
        Serial.println("[SD] Card mounted");
        _sdReady = ensureDirectories();
        if (_sdReady) {
            uint8_t cardType = SD_MMC.cardType();
            uint64_t sizeMB = SD_MMC.cardSize() / (1024ULL * 1024ULL);
            Serial.printf("[SD] Type=%u Size=%lluMB\n", cardType, sizeMB);
        }
    } else {
        Serial.println("[WARN] SD mount failed - offline queue disabled");
        _sdReady = false;
    }
    return _sdReady;
}

bool StorageManager::isReady() const {
    return _sdReady;
}

bool StorageManager::ensureDirectories() {
    if (!SD_MMC.exists(BASE_DIR) && !SD_MMC.mkdir(BASE_DIR)) {
        Serial.println("[WARN] Unable to create base SD directory");
        return false;
    }
    if (!SD_MMC.exists(PENDING_DIR) && !SD_MMC.mkdir(PENDING_DIR)) {
        Serial.println("[WARN] Unable to create pending directory");
        return false;
    }
    if (!SD_MMC.exists(SENT_DIR) && !SD_MMC.mkdir(SENT_DIR)) {
        Serial.println("[WARN] Unable to create sent directory");
        return false;
    }
    Serial.println("[OK] SD ready for offline queue");
    return true;
}

String StorageManager::buildPendingPath() const {
    time_t now = time(nullptr);
    char filename[32];
    if (now > 0) {
        strftime(filename, sizeof(filename), "%Y%m%d_%H%M%S.jpg", localtime(&now));
    } else {
        snprintf(filename, sizeof(filename), "capture_%lu.jpg", millis());
    }
    String path = String(PENDING_DIR) + "/" + filename;
    return path;
}

bool StorageManager::savePendingFrame(const camera_fb_t* fb) {
    if (!_sdReady || !fb || !fb->buf || fb->len == 0) {
        return false;
    }

    String path = buildPendingPath();
    File file = SD_MMC.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("[WARN] Failed to open pending file for write");
        return false;
    }

    size_t written = file.write(fb->buf, fb->len);
    file.close();

    if (written == fb->len) {
        Serial.printf("[QUEUE] Saved image: %s (%u bytes)\n",
                      path.c_str(), fb->len);
        return true;
    }

    Serial.println("[WARN] Partial write to pending file");
    SD_MMC.remove(path);
    return false;
}

bool StorageManager::moveToSent(const String& pendingPath) {
    String fileName = pendingPath.substring(String(PENDING_DIR).length());
    String sentPath = String(SENT_DIR) + fileName;
    if (SD_MMC.rename(pendingPath.c_str(), sentPath.c_str())) {
        return true;
    }
    // Fall back to delete when move fails to avoid re-upload loops.
    SD_MMC.remove(pendingPath);
    return false;
}

void StorageManager::flushPendingQueue(const String& token, UploadManager& uploader) {
    if (!_sdReady) {
        return;
    }
    File dir = SD_MMC.open(PENDING_DIR);
    if (!dir) {
        Serial.println("[WARN] Cannot open pending directory");
        return;
    }

    Serial.println("[QUEUE] Checking pending files on SD...");
    File entry = dir.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) {
            size_t size = entry.size();
            String path = String(entry.path());
            Serial.printf("[QUEUE] Retrying file: %s (%u bytes)\n",
                          path.c_str(), (unsigned)size);

            uint8_t* buffer = (uint8_t*)ps_malloc(size);
            if (!buffer) {
                buffer = (uint8_t*)malloc(size);
            }
            if (!buffer) {
                Serial.println("[WARN] Not enough memory to retry upload");
                entry.close();
                entry = dir.openNextFile();
                continue;
            }

            size_t readBytes = entry.read(buffer, size);
            entry.close();

            if (readBytes != size) {
                Serial.println("[WARN] Failed to read entire pending file");
                free(buffer);
                SD_MMC.remove(path);
                entry = dir.openNextFile();
                continue;
            }

            bool uploaded = uploader.uploadImage(buffer, size, token);
            free(buffer);

            if (uploaded) {
                Serial.println("[OK] Pending file uploaded - moving to /sent");
                moveToSent(path);
            } else {
                Serial.println("[WARN] Upload failed - keeping file in queue");
                // Stop retrying further files this wake to save power
                break;
            }
        } else {
            entry.close();
        }
        entry = dir.openNextFile();
    }
    dir.close();
}
