# ✅ Lỗi Compile Đã Fix!

## 🐛 Vấn đề ban đầu:
```
error: redefinition of 'SleepManager sleepMgr'
```

**Nguyên nhân:** Arduino IDE compile **tất cả file .ino** trong cùng folder  
Có cả `main.ino` (HTTP) và `main_mqtt.ino` (MQTT) → conflict!

---

## ✅ Giải pháp đã áp dụng:

### 1. Đổi tên files:
```
main.ino         → main.ino.bak      (HTTP only - backup)
main_mqtt.ino    → main.ino          (MQTT + HTTP - active)
```

### 2. Kết quả:
```
✅ Chỉ còn 1 file .ino active
✅ Không còn redefinition error
✅ Sẵn sàng compile và upload!
```

---

## 📁 Current Structure

```
examples/modular/main/
├── main.ino              ← ACTIVE (MQTT mode)
├── main.ino.bak          ← BACKUP (HTTP only)
├── config.h              ← Edit WiFi/MQTT here
├── mqtt_manager.h/cpp    ← MQTT implementation
├── wifi_manager.h/cpp
├── auth_manager.h/cpp
├── camera_manager.h/cpp
├── led_manager.h/cpp
├── upload_manager.h/cpp
├── sleep_manager.h/cpp
└── README.md             ← Usage guide
```

---

## 🚀 Next Steps

### 1. Install Libraries (Nếu chưa có)
```
✅ ArduinoJson (6.21.0+)
✅ Adafruit NeoPixel (1.11.0+)
✅ PubSubClient (2.8.0+) ← CHỈ cần cho MQTT
```

See: `LIBRARIES_GUIDE.md`

### 2. Configure WiFi & MQTT
Edit `config.h`:
```cpp
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define MQTT_BROKER "192.168.77.24"  // Your server IP
#define USE_MQTT true
```

### 3. Upload to ESP32
```
Board: ESP32S3 Dev Module
PSRAM: OPI PSRAM ← IMPORTANT!
Upload Speed: 921600
Click: Upload (Ctrl+U)
```

### 4. Monitor Serial
```
Baud Rate: 115200
Should see:
✅ MQTT connected!
✅ Image published to MQTT!
```

---

## 🔄 Switch Modes (Optional)

### To HTTP Only Mode:
```powershell
# In folder: examples/modular/main/
Rename-Item "main.ino" "main_mqtt.ino"
Rename-Item "main.ino.bak" "main.ino"
```

### To MQTT Mode (Current):
Already active! 👍

---

## 📊 File Sizes

```
main.ino (MQTT)      : 5,915 bytes
main.ino.bak (HTTP)  : 4,293 bytes
mqtt_manager.cpp     : 2,455 bytes
Total modular code   : ~28 KB
```

**Memory efficient!** ✅

---

## 🎯 Features của main.ino hiện tại:

- ✅ **MQTT Upload** - Primary method
- ✅ **HTTP Fallback** - Automatic if MQTT fails
- ✅ **Deep Sleep** - ~10µA power
- ✅ **PIR Wake** - Motion triggered
- ✅ **Token Cache** - RTC memory
- ✅ **LED Indicators** - 5 colors
- ✅ **Auto Reconnect** - WiFi & MQTT
- ✅ **Large Images** - 256KB buffer

---

## ✅ Verification

### Check files:
```powershell
Get-ChildItem "*.ino"
# Should show only: main.ino
```

### Check libraries:
```
Arduino IDE → Sketch → Include Library
Should see:
- ArduinoJson ✅
- Adafruit NeoPixel ✅
- PubSubClient ✅
```

### Test compile:
```
Arduino IDE → Verify (Ctrl+R)
Should compile without errors!
```

---

## 🐛 If Still Error

### "PubSubClient.h not found"
**Fix:** Install PubSubClient from Library Manager

### "multiple definition"
**Fix:** Make sure only ONE .ino file exists (rename others to .bak)

### "PSRAM init failed"
**Fix:** Tools → PSRAM → Select "OPI PSRAM"

---

## 📚 Documentation

- `README.md` - Folder guide
- `LIBRARIES_GUIDE.md` - Library installation
- `ESP32_MQTT_UPLOAD.md` - Complete upload guide
- `MQTT_DEPLOYMENT_COMPLETE.md` - System overview

---

**Status:** 🟢 READY TO UPLOAD!

Bây giờ có thể compile và upload lên ESP32 rồi! 🚀
