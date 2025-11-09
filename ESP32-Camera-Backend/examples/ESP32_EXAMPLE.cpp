/**
 * ESP32-S3-EYE Security Camera - Arduino Code Example
 * 
 * Hardware: ESP32-S3-EYE
 * Features:
 * - PIR motion detection
 * - Camera capture
 * - WiFi connectivity
 * - HTTP POST to backend server
 * 
 * Required Libraries:
 * - ESP32 Camera library
 * - WiFi library
 * - HTTPClient library
 * - ArduinoJson library
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// ===== CONFIGURATION =====
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_BASE_URL = "http://192.168.1.100:3000/api";
const char* USERNAME = "Minh Khue";  // Your registered username
const char* USER_PASSWORD = "123456";  // Your password

// JWT Token (will be fetched automatically on startup)
String jwtToken = "";

// PIR Sensor Pin
const int PIR_PIN = 13;  // Adjust based on your wiring

// Motion detection cooldown (milliseconds)
const unsigned long COOLDOWN_PERIOD = 5000;
unsigned long lastMotionTime = 0;

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

// ===== FUNCTION DECLARATIONS =====
void setupCamera();
void connectWiFi();
bool loginAndGetToken();
bool captureAndUploadImage();
void blinkLED(int times);

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32-S3-EYE Security Camera ===");
  
  // Setup PIR sensor
  pinMode(PIR_PIN, INPUT);
  
  // Setup LED (built-in)
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Connect to WiFi
  connectWiFi();
  
  // Login and get JWT token
  if (loginAndGetToken()) {
    Serial.println("✓ Logged in successfully");
    Serial.println("Token: " + jwtToken.substring(0, 20) + "...");
  } else {
    Serial.println("✗ Login failed! Cannot proceed.");
    while(1) {
      blinkLED(10);
      delay(5000);
    }
  }
  
  // Initialize Camera
  setupCamera();
  
  Serial.println("System ready! Monitoring for motion...");
  blinkLED(3);
}

void loop() {
  // Check PIR sensor for motion
  int pirState = digitalRead(PIR_PIN);
  
  if (pirState == HIGH) {
    unsigned long currentTime = millis();
    
    // Check cooldown period
    if (currentTime - lastMotionTime > COOLDOWN_PERIOD) {
      Serial.println("\n🚨 Motion detected!");
      digitalWrite(LED_BUILTIN, HIGH);
      
      // Capture and upload image
      bool success = captureAndUploadImage();
      
      if (success) {
        Serial.println("✓ Image uploaded successfully");
        blinkLED(2);
      } else {
        Serial.println("✗ Upload failed");
        blinkLED(5);
      }
      
      digitalWrite(LED_BUILTIN, LOW);
      lastMotionTime = currentTime;
    }
  }
  
  delay(100);
}

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n✗ WiFi connection failed!");
  }
}

void setupCamera() {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Image quality settings
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
    config.jpeg_quality = 10;             // 0-63 lower means higher quality
    config.fb_count = 2;
    Serial.println("PSRAM found - Using high quality");
  } else {
    config.frame_size = FRAMESIZE_SVGA;   // 800x600
    config.jpeg_quality = 12;
    config.fb_count = 1;
    Serial.println("No PSRAM - Using standard quality");
  }
  
  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("✗ Camera init failed with error 0x%x\n", err);
    return;
  }
  
  // Additional sensor settings
  sensor_t * s = esp_camera_sensor_get();
  if (s != NULL) {
    s->set_brightness(s, 0);     // -2 to 2
    s->set_contrast(s, 0);       // -2 to 2
    s->set_saturation(s, 0);     // -2 to 2
    s->set_special_effect(s, 0); // 0 = No Effect
    s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
    s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
    s->set_wb_mode(s, 0);        // 0 to 4
    s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
    s->set_aec2(s, 0);           // 0 = disable , 1 = enable
    s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
    s->set_agc_gain(s, 0);       // 0 to 30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
    s->set_bpc(s, 0);            // 0 = disable , 1 = enable
    s->set_wpc(s, 1);            // 0 = disable , 1 = enable
    s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
    s->set_lenc(s, 1);           // 0 = disable , 1 = enable
    s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
    s->set_vflip(s, 0);          // 0 = disable , 1 = enable
    s->set_dcw(s, 1);            // 0 = disable , 1 = enable
    s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  }
  
  Serial.println("✓ Camera initialized");
}

bool loginAndGetToken() {
  Serial.println("Logging in to server...");
  
  HTTPClient http;
  String loginUrl = String(SERVER_BASE_URL) + "/auth/login";
  http.begin(loginUrl);
  http.addHeader("Content-Type", "application/json");
  
  // Create JSON payload
  DynamicJsonDocument loginDoc(256);
  loginDoc["username"] = USERNAME;
  loginDoc["password"] = USER_PASSWORD;
  
  String requestBody;
  serializeJson(loginDoc, requestBody);
  
  Serial.println("Login URL: " + loginUrl);
  Serial.println("Request: " + requestBody);
  
  // Send POST request
  int httpResponseCode = http.POST(requestBody);
  
  bool success = false;
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    
    // Parse JSON response
    DynamicJsonDocument responseDoc(2048);
    DeserializationError error = deserializeJson(responseDoc, response);
    
    if (!error) {
      bool loginSuccess = responseDoc["success"];
      if (loginSuccess && responseDoc["data"]["token"]) {
        jwtToken = responseDoc["data"]["token"].as<String>();
        success = true;
        Serial.println("✓ Token received");
      } else {
        const char* message = responseDoc["message"];
        Serial.println("Login failed: " + String(message));
      }
    } else {
      Serial.println("JSON parse error: " + String(error.c_str()));
      Serial.println("Response: " + response);
    }
  } else {
    Serial.printf("✗ HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
  return success;
}

bool captureAndUploadImage() {
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) {
      return false;
    }
  }
  
  // Check if we have a token
  if (jwtToken.length() == 0) {
    Serial.println("No JWT token! Attempting to login...");
    if (!loginAndGetToken()) {
      Serial.println("✗ Failed to get token");
      return false;
    }
  }
  
  // Capture image
  Serial.println("Capturing image...");
  camera_fb_t *fb = esp_camera_fb_get();
  
  if (!fb) {
    Serial.println("✗ Camera capture failed");
    return false;
  }
  
  Serial.printf("Image size: %d bytes\n", fb->len);
  
  // Upload via HTTP POST
  HTTPClient http;
  String uploadUrl = String(SERVER_BASE_URL) + "/upload-image";
  http.begin(uploadUrl);
  http.addHeader("Authorization", "Bearer " + jwtToken);
  http.addHeader("Content-Type", "image/jpeg");
  
  Serial.println("Uploading to server...");
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  bool success = false;
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    Serial.println("Response: " + response);
    
    // Parse JSON response
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (!error) {
      bool responseSuccess = doc["success"];
      const char* message = doc["message"];
      Serial.println(message);
      success = responseSuccess;
      
      // If token expired (401), try to login again
      if (httpResponseCode == 401) {
        Serial.println("Token expired! Re-authenticating...");
        esp_camera_fb_return(fb);
        http.end();
        
        if (loginAndGetToken()) {
          // Retry upload with new token
          return captureAndUploadImage();
        }
        return false;
      }
    }
  } else {
    Serial.printf("✗ HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  // Cleanup
  esp_camera_fb_return(fb);
  http.end();
  
  return success;
}

void blinkLED(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
}

// Optional: Stream server for live view
void startCameraServer() {
  // Implementation depends on your needs
  // Can use ESPAsyncWebServer library for MJPEG streaming
}
