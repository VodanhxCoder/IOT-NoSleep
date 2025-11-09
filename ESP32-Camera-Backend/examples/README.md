# Modular ESP32 Deep Sleep Motion Camera

## 📁 File Structure

```
modular/
├── main.ino                 # Main sketch - orchestrates all modules
├── config.h                 # All configuration constants
├── sleep_manager.h/cpp      # Deep sleep management
├── wifi_manager.h/cpp       # WiFi connection
├── auth_manager.h/cpp       # JWT authentication (RTC cached)
├── led_manager.h/cpp        # LED flash & status
├── camera_manager.h/cpp     # Camera init & capture
└── upload_manager.h/cpp     # HTTP image upload
```

## 🎯 Benefits of Modular Design

### ✅ Advantages:
- **Reduced Memory Load**: Each module compiled separately reduces RAM usage
- **Better Organization**: Clean separation of concerns
- **Easier Debugging**: Test modules independently
- **Maintainable**: Update one module without touching others
- **Reusable**: Use managers in other ESP32 projects

### 📉 Memory Comparison:
- **Monolithic version**: ~500 lines, ~280KB flash, ~45KB RAM
- **Modular version**: 8 files, **~240KB flash, ~38KB RAM** (estimated)

## 🔧 Arduino IDE Setup

### 1. Create Project Folder
```
Documents/Arduino/ESP32_Motion_Camera_Modular/
├── ESP32_Motion_Camera_Modular.ino  (rename main.ino)
├── config.h
├── sleep_manager.h
├── sleep_manager.cpp
├── wifi_manager.h
├── wifi_manager.cpp
├── auth_manager.h
├── auth_manager.cpp
├── led_manager.h
├── led_manager.cpp
├── camera_manager.h
├── camera_manager.cpp
├── upload_manager.h
└── upload_manager.cpp
```

**⚠️ Important**: Arduino requires `.ino` file name to match folder name!

### 2. Required Libraries
Install via Arduino IDE Library Manager:
- **ArduinoJson** by Benoit Blanchon (v6.21+)
- **Adafruit NeoPixel** (v1.11+)

Built-in libraries (no installation needed):
- WiFi
- HTTPClient
- esp_camera
- esp_sleep

### 3. Board Configuration
**Tools > Board**: "ESP32-S3-Dev Module"
**Settings**:
- Flash Size: 8MB
- Partition Scheme: Huge APP (3MB No OTA)
- PSRAM: OPI PSRAM
- Upload Speed: 921600
- USB CDC On Boot: Enabled

## ⚙️ Configuration

Edit `config.h`:

```cpp
// WiFi credentials
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourWiFiPassword"

// Server settings
#define SERVER_IP "192.168.2.22"  // Your server IP
#define SERVER_PORT 3000

// Login credentials
#define LOGIN_USERNAME "Minh Khue"  // Your username
#define LOGIN_PASSWORD "123456"     // Your password
```

## 🚀 Compilation & Upload

### Compile:
```
Sketch > Verify/Compile
```

Expected output:
```
Sketch uses XXXXXX bytes (XX%) of program storage space.
Global variables use XXXXX bytes (XX%) of dynamic memory.
```

### Upload:
```
Sketch > Upload
```

### Monitor Serial:
```
Tools > Serial Monitor (115200 baud)
```

Expected output:
```
🚀 ESP32 Motion Camera Starting
[1/5] Connecting to WiFi...
✅ Connected! IP: 192.168.2.XXX
[2/5] Authenticating...
✅ Authenticated!
[3/5] Initializing camera...
✅ Camera ready!
[4/5] Capturing photo...
✅ Captured! Size: 125638 bytes
[5/5] Uploading to server...
✅ Upload successful!
💤 Entering deep sleep...
```

## 🔍 How It Works

### Startup Sequence:
1. **Wake up** from deep sleep (motion detected on GPIO14)
2. **Connect WiFi** (`wifi_manager`)
3. **Authenticate** - login or use cached token (`auth_manager`)
4. **Initialize camera** with PSRAM detection (`camera_manager`)
5. **Capture photo** with LED flash (`led_manager` + `camera_manager`)
6. **Upload image** to server via HTTP POST (`upload_manager`)
7. **Enter deep sleep** - wait for next motion (`sleep_manager`)

### Power Consumption:
- **Deep Sleep**: ~10µA (6+ months on 3000mAh battery)
- **Active (capture + upload)**: ~160mA for ~5 seconds
- **Average**: Depends on motion frequency

### Token Caching:
- JWT token stored in **RTC memory** (persists through deep sleep)
- Auto-login on first wake or token expired
- No re-login on subsequent wakes (saves time & power)

## 🐛 Troubleshooting

### Camera Init Failed:
```
❌ Camera init failed - sleeping
```
**Solutions**:
- Check GPIO connections (camera pins in `config.h`)
- Verify PSRAM enabled in board settings
- Try power cycle ESP32

### WiFi Connection Failed:
```
❌ WiFi failed - sleeping 30s
```
**Solutions**:
- Check SSID/password in `config.h`
- Verify router is on and reachable
- Check WiFi signal strength

### Auth Failed:
```
❌ Auth failed - sleeping 30s
```
**Solutions**:
- Verify server is running: `http://192.168.2.22:3000`
- Check username/password in `config.h`
- Confirm server IP address

### Upload Failed:
```
❌ Upload failed
```
**Solutions**:
- Check network stability
- Verify server endpoint: `/api/images/upload`
- Look at server logs for errors
- Check JWT token validity

### Red LED Flashing:
- **3 flashes**: WiFi/Auth/Upload error
- **5 flashes**: Camera init error

### Green LED Flashing:
- **1 flash**: WiFi connected
- **2 flashes**: Photo captured
- **3 flashes**: Upload successful

## 📊 Testing Individual Modules

You can test each module independently:

### Test WiFi:
```cpp
void setup() {
    Serial.begin(115200);
    WiFiManager wifiMgr;
    if (wifiMgr.connect()) {
        Serial.println("✅ WiFi OK");
    }
}
```

### Test Auth:
```cpp
void setup() {
    Serial.begin(115200);
    WiFiManager wifiMgr;
    wifiMgr.connect();
    
    AuthManager authMgr;
    if (authMgr.ensureLoggedIn()) {
        Serial.print("Token: ");
        Serial.println(authMgr.getToken());
    }
}
```

### Test Camera:
```cpp
void setup() {
    Serial.begin(115200);
    CameraManager cameraMgr;
    
    if (cameraMgr.init()) {
        camera_fb_t* fb = cameraMgr.capture();
        if (fb) {
            Serial.print("Size: ");
            Serial.println(fb->len);
            cameraMgr.returnFrameBuffer(fb);
        }
    }
}
```

## 🎓 Code Architecture

### Class Responsibilities:

**SleepManager**: Deep sleep control & wake reason detection
- `enterDeepSleep()` - Configure ext0 wake & sleep
- `wokeByMotion()` - Check if woke by PIR
- `enableTimerWake()` - Optional timer backup

**WiFiManager**: Network connectivity
- `connect()` - Connect to WiFi with timeout
- `disconnect()` - Disconnect & save power
- `isConnected()` - Check connection status

**AuthManager**: JWT authentication with RTC caching
- `ensureLoggedIn()` - Login or use cached token
- `getToken()` - Retrieve current token
- `clearToken()` - Force re-login
- Token stored in `RTC_DATA_ATTR` (survives sleep)

**LEDManager**: Visual feedback
- `flashGreen(n)` - Success indicator (n times)
- `flashRed(n)` - Error indicator
- `flashBlue(n)` - Motion detected
- `setFlash(on)` - Camera flash control

**CameraManager**: Image capture
- `init()` - Initialize with PSRAM detection
- `capture()` - Take photo, return frame buffer
- `returnFrameBuffer()` - Free memory
- `deinit()` - Cleanup & release resources

**UploadManager**: HTTP communication
- `uploadImage()` - POST multipart/form-data
- Automatic JWT authorization header
- Server response parsing

## 💡 Customization

### Change Image Quality:
Edit `config.h`:
```cpp
#define CAMERA_QUALITY 10  // 0-63 (lower = higher quality)
#define CAMERA_SIZE FRAMESIZE_UXGA // SVGA, XGA, UXGA, etc.
```

### Add Timer Wake (backup):
In `main.ino`:
```cpp
sleepMgr.enableTimerWake(3600 * 1000000); // Wake every hour
sleepMgr.enterDeepSleep();
```

### Change LED Pin:
Edit `config.h`:
```cpp
#define LED_PIN 48  // Change to your LED GPIO
#define NUM_LEDS 1  // Number of WS2812 LEDs
```

### Add SD Card Backup:
Create `sd_manager.h/cpp` based on commented code in original `ESP32_MOTION_DEEPSLEEP.cpp`

## 📝 License & Credits

Created for ESP32-S3-EYE with PIR motion sensor
Optimized for ultra-low power deep sleep operation
Modular design for better memory management

**Hardware**: ESP32-S3-EYE, PIR sensor (GPIO14), WS2812 LED (GPIO48)
**Backend**: Node.js + Express + MongoDB
**Frontend**: React + Vite + Tailwind CSS
