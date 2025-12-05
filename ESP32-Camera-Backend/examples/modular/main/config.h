/**
 * config.h - Central configuration for ESP32 firmware
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== WIFI & SERVER =====
#define WIFI_SSID "..."
#define WIFI_PASSWORD "20041610"

// Server Configuration
#define SERVER_HOSTNAME_MDNS "esp32-server" // Hostname to search for via mDNS
extern char serverIP[16];                   // Global variable to store resolved IP

#define SERVER_PORT 3000
#define SERVER_API_PATH "/api"

#define USERNAME "MinhKhue123"
#define USER_PASSWORD "123456"

// ===== IMAGE ENCRYPTION =====
#define IMAGE_SECRET_KEY "my_super_secret_key_123"  // 16 chars for AES-128

// ===== MQTT CONFIG =====
#define USE_MQTT true
#define MQTT_FOR_IMAGES false
#define MQTT_BROKER "2eff9ea6be6845b793f43e33c10067e0.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USERNAME "esp32-cam"
#define MQTT_PASSWORD "Khueqp123"
#define MQTT_CLIENT_ID "ESP32-CAM-001"
#define MQTT_TOPIC_IMAGE "esp32/camera/image"
#define MQTT_TOPIC_STATUS "esp32/camera/status"
#define MQTT_TOPIC_COMMAND "esp32/camera/command" // New command topic

// ===== STREAMING CONFIG =====
// Set to true to enable MJPEG streaming (DISABLES DEEP SLEEP)
#define ENABLE_STREAMING_MODE true  
#define STREAM_PORT 81

// ===== HARDWARE PINS =====
#define USE_PIR         true    // Set to false to disable PIR sensor logic completely
#define PIR_PIN         0       // GPIO 0
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

// ===== SHARED STATE =====
extern volatile bool pauseStreamForCapture;
extern volatile bool captureRequested; // New flag for stream-based capture
extern volatile bool isStreaming;      // Track streaming state globally

// ===== IMAGE QUALITY =====
// Optimized for smoother streaming (VGA 640x480)
#define FRAME_SIZE_HIGH     FRAMESIZE_VGA
#define JPEG_QUALITY_HIGH   20
#define FB_COUNT_HIGH       2

#define FRAME_SIZE_STD      FRAMESIZE_VGA
#define JPEG_QUALITY_STD    20
#define FB_COUNT_STD        1

#endif // CONFIG_H
