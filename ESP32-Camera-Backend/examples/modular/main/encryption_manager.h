/**
 * encryption_manager.h - AES encryption utilities for image upload
 */

#ifndef ENCRYPTION_MANAGER_H
#define ENCRYPTION_MANAGER_H

#include <Arduino.h>
#include "config.h"

struct EncryptionResult {
    uint8_t* data;
    size_t length;
    char ivBase64[25];

    EncryptionResult() : data(nullptr), length(0) {
        ivBase64[0] = '\0';
    }
};

class EncryptionManager {
public:
    EncryptionManager();
    bool encrypt(const uint8_t* input, size_t len, EncryptionResult& result);
    void freeResult(EncryptionResult& result);

private:
    void deriveKey(uint8_t* keyOut);
    bool base64Encode(const uint8_t* input, size_t len, char* output, size_t outputSize);
};

#endif // ENCRYPTION_MANAGER_H
