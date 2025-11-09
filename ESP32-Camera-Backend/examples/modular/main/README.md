# ESP32 Modular Code - MQTT Version

## 📁 Files Overview

### ✅ Active Files (MQTT Mode)
- `main.ino` - **MQTT + HTTP Fallback** (Active - sẽ compile)
- `mqtt_manager.h/cpp` - MQTT connection & publishing
- All other manager files (wifi, auth, camera, etc.)

### 📦 Backup Files
- `main.ino.bak` - **HTTP Only version** (Backup)

## 🔄 Switch Between Modes

### Use MQTT Mode (Current):
```
main.ino = MQTT + HTTP Fallback ✅
```
Already active! Just upload.

### Switch to HTTP Only Mode:
```powershell
# In folder: examples/modular/main/
Rename-Item "main.ino" "main_mqtt.ino"
Rename-Item "main.ino.bak" "main.ino"
```

## 🚀 Upload Instructions

### 1. Install Library
Arduino IDE → Library Manager → Install **PubSubClient**

### 2. Configure WiFi & MQTT
Edit `config.h`:
```cpp
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define MQTT_BROKER "192.168.77.24"  // Your server IP
#define USE_MQTT true
```

### 3. Upload
- Board: ESP32S3 Dev Module
- PSRAM: OPI PSRAM
- Upload Speed: 921600
- Click Upload (Ctrl+U)

### 4. Monitor
Open Serial Monitor (115200 baud) to see:
```
🚀 ESP32 Motion Camera Starting
   MODE: MQTT + HTTP Fallback
✅ MQTT connected!
✅ Image published to MQTT!
```

## 📊 Features

### MQTT Mode (main.ino):
- ✅ Publish images via MQTT
- ✅ Auto fallback to HTTP if MQTT fails
- ✅ 256KB buffer for large images
- ✅ Real-time status updates
- ✅ Auto reconnection
- ✅ LED indicators (5 colors)

### HTTP Only Mode (main.ino.bak):
- ✅ Direct HTTP upload
- ✅ Multipart/form-data
- ✅ Simpler, more stable
- ✅ No MQTT dependency

## 🎯 Recommendations

**Use MQTT Mode if:**
- ✅ Want real-time updates
- ✅ Need faster upload
- ✅ Backend MQTT is running
- ✅ Want automatic fallback

**Use HTTP Only Mode if:**
- ✅ MQTT not needed
- ✅ Simpler deployment
- ✅ Less dependencies
- ✅ Direct upload preferred

## 🔧 Troubleshooting

### Compile Error: "redefinition of..."
**Cause:** Multiple .ino files in same folder  
**Fix:** Only keep ONE .ino file (rename others to .bak)

### "PubSubClient.h: No such file"
**Fix:** Install PubSubClient library from Library Manager

### MQTT connection failed
**Fix:** Check `MQTT_BROKER` IP in config.h matches your server

## 📚 More Info

See: `ESP32_MQTT_UPLOAD.md` for complete guide
