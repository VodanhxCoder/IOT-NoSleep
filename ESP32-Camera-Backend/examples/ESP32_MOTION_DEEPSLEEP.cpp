/**
 * ESP32-S3-EYE Security Camera - Motion Detection with Deep Sleep
 * 
 * Features:
 * - PIR motion detection with deep sleep (low power)
 * - Auto wake on motion -> capture -> upload to server -> deep sleep
 * - Auto login and JWT token management
 * - SD card backup (optional)
 * - WS2812 LED flash
 * 
 * Hardware:
 * - ESP32-S3-EYE
 * - PIR sensor on GPIO14 (RTC-capable)
 * - WS2812 LED on GPIO48
 * - SD card (optional backup)
 * 
 * Required Libraries:
 * - ESP32 Camera
 * - WiFi, HTTPClient
 * - ArduinoJson
 * - Adafruit_NeoPixel
 * - SD_MMC (optional)
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "esp_sleep.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <Adafruit_NeoPixel.h>
// Uncomment if using SD card backup
// #include "FS.h"
// #include "SD_MMC.h"

// ===== WIFI & SERVER CONFIGURATION =====
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_BASE_URL = "http://192.168.2.22:3000/api";
const char* USERNAME = "Minh Khue";
const char* USER_PASSWORD = "123456";

// JWT Token (will be fetched automatically)
String jwtToken = "";

// ===== HARDWARE PINS =====
#define PIR_PIN         14    // RTC-capable pin for ext0 wake
#define WS2812_PIN      48    // NeoPixel LED
#define WS2812_COUNT    1

// SD_MMC pins (uncomment if using SD backup)
// #define SD_MMC_CLK  GPIO_NUM_39
// #define SD_MMC_CMD  GPIO_NUM_38
// #define SD_MMC_D0   GPIO_NUM_40

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
const uint32_t FLASH_DURATION_MS = 150;       // Flash duration
const uint32_t POST_UPLOAD_DELAY_MS = 2000;  // Delay before sleep
const uint32_t WIFI_TIMEOUT_MS = 15000;      // WiFi connection timeout

// ===== GLOBALS =====
Adafruit_NeoPixel pixels(WS2812_COUNT, WS2812_PIN, NEO_GRB + NEO_KHZ800);
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR char savedToken[512] = "";  // Store token in RTC memory

// ===== FUNCTION DECLARATIONS =====
void connectWiFi();
bool loginAndGetToken();
bool initCamera();
void flashLED(uint32_t ms, uint8_t r, uint8_t g, uint8_t b);
bool captureAndUpload();
void enterDeepSleep();
void blinkError(int times);

// ===== SETUP =====
void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  delay(200);
  Serial.println("\n\n=== ESP32-S3-EYE Motion Detection System ===");
  
  // Increment boot count
  bootCount++;
  Serial.printf("Boot count: %d\n", bootCount);
  
  // Init LED
  pixels.begin();
  pixels.clear();
  pixels.show();
  
  // Setup PIR pin
  pinMode(PIR_PIN, INPUT);
  
  // Get wake reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("🚨 Woke by PIR motion detection!");
      flashLED(100, 0, 255, 0); // Green flash
      
      // Connect WiFi
      connectWiFi();
      
      // Restore or fetch token
      if (strlen(savedToken) > 0) {
        jwtToken = String(savedToken);
        Serial.println("✓ Token restored from RTC memory");
      } else {
        Serial.println("No saved token, logging in...");
        if (!loginAndGetToken()) {
          Serial.println("✗ Login failed!");
          blinkError(5);
          enterDeepSleep();
          return;
        }
        // Save token to RTC memory
        strncpy(savedToken, jwtToken.c_str(), sizeof(savedToken) - 1);
      }
      
      // Init camera and capture
      if (!initCamera()) {
        Serial.println("✗ Camera init failed!");
        blinkError(3);
        enterDeepSleep();
        return;
      }
      
      // Capture and upload
      if (captureAndUpload()) {
        Serial.println("✓ Success! Image uploaded.");
        flashLED(200, 0, 255, 0); // Green success
      } else {
        Serial.println("✗ Upload failed, retrying login...");
        // Token might be expired, try re-login
        if (loginAndGetToken()) {
          strncpy(savedToken, jwtToken.c_str(), sizeof(savedToken) - 1);
          // Retry upload
          if (captureAndUpload()) {
            Serial.println("✓ Retry successful!");
            flashLED(200, 0, 255, 0);
          } else {
            blinkError(5);
          }
        } else {
          blinkError(10);
        }
      }
      
      // Cleanup
      esp_camera_deinit();
      WiFi.disconnect(true);
      
      // Wait before sleep
      delay(POST_UPLOAD_DELAY_MS);
      break;
      
    default:
      Serial.println("🔌 Initial boot - configuring wake on motion");
      flashLED(300, 0, 0, 255); // Blue flash
      break;
  }
  
  // Enter deep sleep with ext0 wake on PIR
  enterDeepSleep();
}

void loop() {
  // Never reached because we always deep sleep
}

// ===== WIFI CONNECTION =====
void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > WIFI_TIMEOUT_MS) {
      Serial.println("\n✗ WiFi timeout!");
      return;
    }
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n✓ WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ===== LOGIN AND GET JWT TOKEN =====
bool loginAndGetToken() {
  Serial.println("Logging in to server...");
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    return false;
  }
  
  HTTPClient http;
  String loginUrl = String(SERVER_BASE_URL) + "/auth/login";
  http.begin(loginUrl);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000); // 10s timeout
  
  // Create JSON payload
  DynamicJsonDocument loginDoc(256);
  loginDoc["username"] = USERNAME;
  loginDoc["password"] = USER_PASSWORD;
  
  String requestBody;
  serializeJson(loginDoc, requestBody);
  
  Serial.println("POST " + loginUrl);
  
  // Send request
  int httpCode = http.POST(requestBody);
  bool success = false;
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP %d\n", httpCode);
    
    if (httpCode == 200) {
      DynamicJsonDocument responseDoc(2048);
      DeserializationError error = deserializeJson(responseDoc, response);
      
      if (!error && responseDoc["success"] && responseDoc["data"]["token"]) {
        jwtToken = responseDoc["data"]["token"].as<String>();
        Serial.println("✓ Token received");
        success = true;
      } else {
        Serial.println("JSON parse error or no token");
      }
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  http.end();
  return success;
}

// ===== INIT CAMERA =====
bool initCamera() {
  Serial.println("Initializing camera...");
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  
  // Quality settings
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
    Serial.println("PSRAM found - High quality mode");
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
    Serial.println("No PSRAM - Standard quality mode");
  }
  
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("✗ Camera init failed: 0x%x\n", err);
    return false;
  }
  
  // Sensor settings
  sensor_t * s = esp_camera_sensor_get();
  if (s) {
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_vflip(s, 0);
    s->set_hmirror(s, 0);
  }
  
  Serial.println("✓ Camera ready");
  return true;
}

// ===== CAPTURE AND UPLOAD =====
bool captureAndUpload() {
  Serial.println("📸 Capturing image...");
  
  // Flash before capture
  flashLED(FLASH_DURATION_MS, 255, 255, 255); // White flash
  
  // Capture
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("✗ Capture failed");
    return false;
  }
  
  Serial.printf("Image size: %u bytes (%dx%d)\n", fb->len, fb->width, fb->height);
  
  // Upload to server
  HTTPClient http;
  String uploadUrl = String(SERVER_BASE_URL) + "/upload-image";
  http.begin(uploadUrl);
  http.addHeader("Authorization", "Bearer " + jwtToken);
  http.addHeader("Content-Type", "image/jpeg");
  http.setTimeout(30000); // 30s timeout for upload
  
  Serial.println("📤 Uploading to server...");
  int httpCode = http.POST(fb->buf, fb->len);
  
  bool success = false;
  if (httpCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP %d\n", httpCode);
    
    if (httpCode == 200 || httpCode == 201) {
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, response);
      
      if (!error && doc["success"]) {
        const char* message = doc["message"] | "Success";
        Serial.println(message);
        success = true;
      }
    } else if (httpCode == 401) {
      Serial.println("Token expired (401)");
      // Clear saved token
      savedToken[0] = '\0';
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpCode).c_str());
  }
  
  // Cleanup
  esp_camera_fb_return(fb);
  http.end();
  
  return success;
}

// ===== LED FLASH =====
void flashLED(uint32_t ms, uint8_t r, uint8_t g, uint8_t b) {
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
  delay(ms);
  pixels.clear();
  pixels.show();
}

// ===== ERROR BLINK =====
void blinkError(int times) {
  for (int i = 0; i < times; i++) {
    flashLED(200, 255, 0, 0); // Red
    delay(200);
  }
}

// ===== ENTER DEEP SLEEP =====
void enterDeepSleep() {
  Serial.println("\n💤 Configuring deep sleep...");
  Serial.println("Wake trigger: PIR motion on GPIO14 (HIGH)");
  
  // Configure ext0 wake on PIR_PIN going HIGH
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, 1);
  
  // Optional: Timer wake as backup (every 1 hour)
  // esp_sleep_enable_timer_wakeup(3600 * 1000000ULL); // 1 hour in microseconds
  
  Serial.println("Entering deep sleep NOW...");
  Serial.flush();
  delay(100);
  
  // Deep sleep
  esp_deep_sleep_start();
  
  // Never reaches here
}

/*
 * ===== OPTIONAL: SD CARD BACKUP =====
 * Uncomment below functions if you want to save images to SD card
 * Also uncomment SD_MMC includes at the top
 */

/*
bool initSD() {
  Serial.println("Mounting SD card...");
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (!SD_MMC.begin("/sd", true)) {
    Serial.println("SD mount failed");
    return false;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card");
    return false;
  }
  
  Serial.println("✓ SD card mounted");
  return true;
}

bool saveToSD(camera_fb_t *fb) {
  if (!SD_MMC.begin()) {
    return false;
  }
  
  // Create folder
  if (!SD_MMC.exists("/camera")) {
    SD_MMC.mkdir("/camera");
  }
  
  // Generate filename
  int photoIndex = 1;
  File root = SD_MMC.open("/camera");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      String name = String(file.name());
      if (name.startsWith("/camera/photo_") && name.endsWith(".jpg")) {
        String num = name.substring(14, name.length() - 4);
        int v = num.toInt();
        if (v >= photoIndex) photoIndex = v + 1;
      }
      file = root.openNextFile();
    }
    root.close();
  }
  
  char path[64];
  snprintf(path, sizeof(path), "/camera/photo_%04d.jpg", photoIndex);
  
  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return false;
  }
  
  size_t written = file.write(fb->buf, fb->len);
  file.close();
  
  if (written == fb->len) {
    Serial.printf("✓ Saved to SD: %s\n", path);
    return true;
  }
  
  return false;
}
*/
