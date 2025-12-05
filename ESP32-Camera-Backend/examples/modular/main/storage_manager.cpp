/**
 * storage_manager.cpp
 * Implements SD-based offline queue for captured images.
 */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
        _lastPath = path; // Store path
        return true;
    }
    return false;
}

// New method to move file from pending to sent
bool StorageManager::moveToSent(const String& pendingPath) {
    if (!_sdReady) return false;
    
    String filename = pendingPath.substring(pendingPath.lastIndexOf('/') + 1);
    String sentPath = String(SENT_DIR) + "/" + filename;
    
    if (SD_MMC.rename(pendingPath, sentPath)) {
        Serial.printf("[SD] Moved to sent: %s\n", sentPath.c_str());
        return true;
    } else {
        Serial.println("[SD] Failed to move file to sent");
        return false;
    }
}

bool StorageManager::hasPending() {
    if (!_sdReady) return false;
    File root = SD_MMC.open(PENDING_DIR);
    if (!root || !root.isDirectory()) return false;
    
    File file = root.openNextFile();
    bool hasFile = (file == true);
    file.close();
    root.close();
    return hasFile;
}



time_t StorageManager::timestampFromFilename(const String& path) const {
    const char* name = path.c_str();
    const char* base = strrchr(name, '/');
    base = base ? base + 1 : name;
    int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;
    if (sscanf(base, "%4d%2d%2d_%2d%2d%2d", &y, &M, &d, &h, &m, &s) == 6) {
        struct tm tmTime = {};
        tmTime.tm_year = y - 1900;
        tmTime.tm_mon = M - 1;
        tmTime.tm_mday = d;
        tmTime.tm_hour = h;
        tmTime.tm_min = m;
        tmTime.tm_sec = s;
        time_t ts = mktime(&tmTime);
        return ts;
    }
    return 0;
}

bool StorageManager::getPendingSummary(PendingSummary& summary) {
    summary = PendingSummary();
    if (!_sdReady) {
        return false;
    }
    File dir = SD_MMC.open(PENDING_DIR);
    if (!dir) {
        return false;
    }

    File entry = dir.openNextFile();
    while (entry) {
        if (!entry.isDirectory()) {
            summary.count++;
            String path = String(entry.path());
            time_t ts = entry.getLastWrite();
            if (ts == 0) {
                ts = timestampFromFilename(path);
            }
            if (ts > 0) {
                if (summary.oldestTimestamp == 0 || ts < summary.oldestTimestamp) {
                    summary.oldestTimestamp = ts;
                }
                if (ts > summary.latestTimestamp) {
                    summary.latestTimestamp = ts;
                }
            }
        }
        entry.close();
        entry = dir.openNextFile();
    }
    dir.close();
    return summary.count > 0;
}

size_t StorageManager::flushPendingQueue(const String& token,
                                         UploadManager& uploader,
                                         size_t maxFiles,
                                         PendingUploadCallback onFileStart) {
    if (!_sdReady || maxFiles == 0) {
        return 0;
    }
    File dir = SD_MMC.open(PENDING_DIR);
    if (!dir) {
        Serial.println("[WARN] Cannot open pending directory");
        return 0;
    }

    Serial.println("[QUEUE] Checking pending files on SD...");
    size_t uploadedCount = 0;
    File entry = dir.openNextFile();
    while (entry) {
        if (uploadedCount >= maxFiles) {
            entry.close();
            break;
        }

        if (!entry.isDirectory()) {
            size_t size = entry.size();
            String path = String(entry.path());
            Serial.printf("[QUEUE] Retrying file: %s (%u bytes)\n",
                          path.c_str(), (unsigned)size);

            if (size == 0) {
                Serial.println("[QUEUE] Removing zero-byte pending file");
                entry.close();
                SD_MMC.remove(path);
                entry = dir.openNextFile();
                continue;
            }

            if (onFileStart) {
                onFileStart(uploadedCount, path);
            }

            entry.close();

            File fileToUpload = SD_MMC.open(path, FILE_READ);
            if (!fileToUpload) {
                Serial.println("[WARN] Failed to re-open file for upload");
                entry = dir.openNextFile();
                continue;
            }

            size_t fileSize = fileToUpload.size();
            if (fileSize == 0) {
                Serial.println("[WARN] Pending file empty after reopen - deleting");
                fileToUpload.close();
                SD_MMC.remove(path);
                entry = dir.openNextFile();
                continue;
            }

            uint8_t* buffer = (uint8_t*)ps_malloc(fileSize);
            if (!buffer) {
                buffer = (uint8_t*)malloc(fileSize);
            }

            if (!buffer) {
                Serial.println("[WARN] Insufficient memory to upload pending file");
                fileToUpload.close();
                // Stop processing more files; try next wake when memory available
                break;
            }

            size_t readBytes = fileToUpload.read(buffer, fileSize);
            fileToUpload.close();

            if (readBytes != fileSize) {
                Serial.println("[WARN] Failed to read full pending file into memory");
                free(buffer);
                entry = dir.openNextFile();
                continue;
            }

            bool uploaded = uploader.uploadImage(buffer, fileSize, token);
            free(buffer);

            if (uploaded) {
                Serial.println("[OK] Pending file uploaded (streamed) - moving to /sent");
                moveToSent(path);
                uploadedCount++;
            } else {
                Serial.println("[WARN] Upload failed (streamed) - keeping file in queue");
                // Stop retrying further files this wake to save power
                break;
            }
        } else {
            entry.close();
        }
        entry = dir.openNextFile();
    }
    dir.close();
    return uploadedCount;
}
