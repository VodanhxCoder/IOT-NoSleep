# 📦 Required Arduino Libraries

## ✅ Installation Checklist

### Built-in Libraries (No install needed):
- ✅ WiFi (ESP32 core)
- ✅ HTTPClient (ESP32 core)
- ✅ esp_camera (ESP32 core)

### Must Install from Library Manager:

#### 1. ArduinoJson
```
Name: ArduinoJson
Author: Benoit Blanchon
Version: 6.21.0 or newer
```
**How to install:**
1. Arduino IDE → `Sketch` → `Include Library` → `Manage Libraries...`
2. Search: "ArduinoJson"
3. Click Install

#### 2. Adafruit NeoPixel
```
Name: Adafruit NeoPixel
Author: Adafruit
Version: 1.11.0 or newer
```
**How to install:**
1. Library Manager
2. Search: "Adafruit NeoPixel"
3. Click Install
4. Install all dependencies if prompted

#### 3. PubSubClient (For MQTT only)
```
Name: PubSubClient
Author: Nick O'Leary
Version: 2.8.0 or newer
```
**How to install:**
1. Library Manager
2. Search: "PubSubClient"
3. Click Install

> ⚠️ **Note:** If using HTTP-only mode (main.ino.bak), you don't need PubSubClient

## 🔍 Verify Installation

After installing, your libraries should show:
```
Arduino/libraries/
├── ArduinoJson/
├── Adafruit_NeoPixel/
└── PubSubClient/
```

Check: `Sketch` → `Include Library` → Should see all 3 libraries listed

## 🚀 Board Configuration

### ESP32S3 Dev Module Settings:
```
Board: "ESP32S3 Dev Module"
USB CDC On Boot: "Enabled"
CPU Frequency: "240MHz (WiFi)"
Flash Mode: "QIO 80MHz"
Flash Size: "8MB (64Mb)"
Partition Scheme: "8M with spiffs"
PSRAM: "OPI PSRAM"  ← IMPORTANT!
Upload Speed: "921600"
```

### Why PSRAM is Critical:
- Camera needs large buffers
- High-res images (UXGA 1600x1200)
- Without PSRAM → will use lower resolution

## ✅ Quick Test

After installing libraries, compile this test sketch:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <PubSubClient.h>

void setup() {
  Serial.begin(115200);
  Serial.println("✅ All libraries loaded!");
}

void loop() {}
```

If compiles successfully → All libraries OK!

## 🐛 Common Issues

### Issue: "ArduinoJson.h: No such file or directory"
**Fix:** Install ArduinoJson from Library Manager

### Issue: "Adafruit_NeoPixel.h: No such file"
**Fix:** Install Adafruit NeoPixel library

### Issue: "PubSubClient.h: No such file"
**Fix:** Install PubSubClient library (or use HTTP-only mode)

### Issue: "psramInit failed"
**Fix:** Enable PSRAM in board settings:
- Tools → PSRAM → "OPI PSRAM"

### Issue: "Camera init failed"
**Fix:** 
1. Check PSRAM enabled
2. Reset ESP32 (EN button)
3. Check camera connections

## 📱 Ready to Upload?

Once all libraries installed:
1. ✅ Open `main.ino`
2. ✅ Edit `config.h` (WiFi, MQTT settings)
3. ✅ Select board: ESP32S3 Dev Module
4. ✅ Enable PSRAM: OPI PSRAM
5. ✅ Click Upload (Ctrl+U)
6. ✅ Open Serial Monitor (115200)

See `ESP32_MQTT_UPLOAD.md` for full guide!
