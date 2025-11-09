/**
 * config.h - Configuration file
 * All system configurations in one place
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== WIFI & SERVER CONFIGURATION =====
#define WIFI_SSID "..."
#define WIFI_PASSWORD "20041610"

// ===== SERVER IP CONFIGURATION =====
// Option 1: Use mDNS hostname (auto-discover, no IP changes needed!)
#define USE_MDNS true
#define SERVER_HOSTNAME "esp32-server.local"
#define SERVER_IP "192.168.77.24"               // Fallback if mDNS fails

#define SERVER_BASE_URL "http://esp32-server.local:3000/api"

#define USERNAME "MinhKhue123"
#define USER_PASSWORD "123456"

// ===== MQTT CONFIGURATION =====
// IMPORTANT: MQTT for images has reliability issues (42KB images fail)
// Recommendation: Use HTTP for images, MQTT for status/notifications only
#define USE_MQTT false                          // Set to true only for small messages
#define MQTT_FOR_IMAGES false                   // Keep false - use HTTP for images
#define MQTT_BROKER "192.168.77.24"             // ← PC WiFi IP
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32-CAM-001"
#define MQTT_TOPIC_IMAGE "esp32/camera/image"
#define MQTT_TOPIC_STATUS "esp32/camera/status"
// Optional: MQTT authentication (if enabled on broker)
// #define MQTT_USERNAME "esp32user"
// #define MQTT_PASSWORD "your-password"

// ===== HARDWARE PINS =====
#define PIR_PIN         14    // RTC-capable pin for ext0 wake
#define WS2812_PIN      48    // NeoPixel LED
#define WS2812_COUNT    1

// ===== HTTP STREAM SERVER =====
#define ENABLE_STREAM_SERVER true              // Enable MJPEG streaming
#define STREAM_SERVER_PORT 81                  // HTTP server port for streaming
#define STREAM_FPS 15                          // Frames per second for stream

// SD_MMC pins (uncomment if using SD backup)
#define SD_MMC_CLK  39
#define SD_MMC_CMD  38
#define SD_MMC_D0   40

// ===== ESP32-S3-EYE CAMERA PINS =====
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

// ===== TIMING CONFIGURATION =====
#define FLASH_DURATION_MS 150
#define POST_UPLOAD_DELAY_MS 2000
#define WIFI_TIMEOUT_MS 15000

// ===== IMAGE QUALITY =====
// For MQTT: Keep images under 100KB for reliability
// Higher quality number = Lower quality = Smaller size

// MQTT-optimized settings (PSRAM)
#define FRAME_SIZE_HIGH     FRAMESIZE_SXGA    // 1280x1024 (smaller than UXGA)
#define JPEG_QUALITY_HIGH   15                // Was 10, now 15 for smaller size
#define FB_COUNT_HIGH       2

// Standard quality settings (No PSRAM)
#define FRAME_SIZE_STD      FRAMESIZE_SVGA    // 800x600
#define JPEG_QUALITY_STD    12
#define FB_COUNT_STD        1

// Note: UXGA (1600x1200) @ quality 10 = ~70KB
//       SXGA (1280x1024) @ quality 15 = ~40-50KB (better for MQTT)
//       To use max quality, set USE_MQTT=false and use HTTP only

#endif // CONFIG_H
