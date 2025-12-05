/**
 * encryption_manager.cpp - AES-128-CBC encryption helper
 */

#include "encryption_manager.h"
#include "config.h"

#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <esp_system.h>
#include <string.h>

EncryptionManager::EncryptionManager() {}

bool EncryptionManager::encrypt(const uint8_t* input, size_t len, EncryptionResult& result) {
    if (!input || len == 0) {
        return false;
    }

    const size_t blockSize = 16;
    size_t paddedLen = ((len / blockSize) + 1) * blockSize;
    uint8_t* buffer = (uint8_t*)malloc(paddedLen);
    if (!buffer) {
        return false;
    }

    memcpy(buffer, input, len);
    uint8_t padValue = paddedLen - len;
    memset(buffer + len, padValue, padValue);

    uint8_t key[blockSize];
    deriveKey(key);

    uint8_t iv[blockSize];
    for (size_t i = 0; i < blockSize; i++) {
        iv[i] = (uint8_t)(esp_random() & 0xFF);
    }

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    int rc = mbedtls_aes_setkey_enc(&aes, key, 128);
    if (rc != 0) {
        mbedtls_aes_free(&aes);
        free(buffer);
        return false;
    }

    uint8_t ivCopy[blockSize];
    memcpy(ivCopy, iv, blockSize);
    rc = mbedtls_aes_crypt_cbc(&aes, MBEDTLS_AES_ENCRYPT, paddedLen, ivCopy, buffer, buffer);
    mbedtls_aes_free(&aes);

    if (rc != 0) {
        free(buffer);
        return false;
    }

    if (!base64Encode(iv, blockSize, result.ivBase64, sizeof(result.ivBase64))) {
        free(buffer);
        return false;
    }

    result.data = buffer;
    result.length = paddedLen;
    return true;
}

void EncryptionManager::freeResult(EncryptionResult& result) {
    if (result.data) {
        free(result.data);
        result.data = nullptr;
    }
    result.length = 0;
    result.ivBase64[0] = '\0';
}

void EncryptionManager::deriveKey(uint8_t* keyOut) {
    memset(keyOut, 0, 16);
    const char* secret = IMAGE_SECRET_KEY;
    size_t secretLen = strlen(secret);
    if (secretLen > 16) secretLen = 16;
    memcpy(keyOut, secret, secretLen);
}

bool EncryptionManager::base64Encode(const uint8_t* input, size_t len, char* output, size_t outputSize) {
    size_t outLen = 0;
    int rc = mbedtls_base64_encode(reinterpret_cast<unsigned char*>(output), outputSize, &outLen, input, len);
    if (rc != 0 || outLen + 1 > outputSize) {
        return false;
    }
    output[outLen] = '\0';
    return true;
}
