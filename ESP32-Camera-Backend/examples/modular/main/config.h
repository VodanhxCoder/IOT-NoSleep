/**
 * config.h - Central configuration for ESP32 firmware
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== WIFI & SERVER =====
#define WIFI_SSID "..."
#define WIFI_PASSWORD "20041610"

#define SERVER_IP "192.168.2.22"                 // fallback IP
#define SERVER_HOSTNAME "DESKTOP-1GSPB77.local"  // mDNS hostname
#define SERVER_PORT 3000
#define SERVER_API_PATH "/api"

#define USERNAME "MinhKhue123"
#define USER_PASSWORD "123456"

// ===== IMAGE ENCRYPTION =====
#define IMAGE_SECRET_KEY "my_super_secret_key_123"  // 16 chars for AES-128

// ===== MQTT CONFIG =====
#define USE_MQTT true
#define MQTT_FOR_IMAGES false
#define MQTT_BROKER "DESKTOP-1GSPB77.local"
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32-CAM-001"
#define MQTT_TOPIC_IMAGE "esp32/camera/image"
#define MQTT_TOPIC_STATUS "esp32/camera/status"

// ===== HARDWARE PINS =====
#define PIR_PIN         0
#define WS2812_PIN      48
#define WS2812_COUNT    1

#define STATUS_LED_PIN          20
#define STATUS_LED_ACTIVE_LOW   0

#define SD_MMC_CLK  GPIO_NUM_39
#define SD_MMC_CMD  GPIO_NUM_38
#define SD_MMC_D0   GPIO_NUM_40
#define SD_MMC_D1   GPIO_NUM_41
#define SD_MMC_D2   GPIO_NUM_42
#define SD_MMC_D3   GPIO_NUM_37

// ===== CAMERA PINS (ESP32-S3-EYE) =====
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM    4
#define SIOC_GPIO_NUM    5
#define Y9_GPIO_NUM      16
#define Y8_GPIO_NUM      17
#define Y7_GPIO_NUM      18
#define Y6_GPIO_NUM      12
#define Y5_GPIO_NUM      10
#define Y4_GPIO_NUM      8
#define Y3_GPIO_NUM      9
#define Y2_GPIO_NUM      11
#define VSYNC_GPIO_NUM   6
#define HREF_GPIO_NUM    7
#define PCLK_GPIO_NUM    13

// ===== TIMING =====
#define FLASH_DURATION_MS 150
#define POST_UPLOAD_DELAY_MS 2000
#define WIFI_TIMEOUT_MS 15000
#define WIFI_MAX_ATTEMPTS 5
#define WIFI_RETRY_DELAY_MS 2000
#define PIR_WAKE_COOLDOWN_SECONDS 15

// ===== IMAGE QUALITY =====
#define FRAME_SIZE_HIGH     FRAMESIZE_SXGA
#define JPEG_QUALITY_HIGH   15
#define FB_COUNT_HIGH       2

#define FRAME_SIZE_STD      FRAMESIZE_SVGA
#define JPEG_QUALITY_STD    12
#define FB_COUNT_STD        1

#endif // CONFIG_H
