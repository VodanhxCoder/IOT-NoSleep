# 📱 ESP32 MQTT Upload Guide

## 🎯 Tổng quan

Bạn có **2 phiên bản code** để lựa chọn:

1. **main.ino** - HTTP Only (đã test, chạy OK)
2. **main_mqtt.ino** - MQTT + HTTP Fallback (mới, khuyên dùng!)

---

## ✅ Yêu cầu

### Thư viện Arduino (Phải cài):
```
- WiFi (built-in ESP32)
- HTTPClient (built-in ESP32)
- ArduinoJson (6.21.0+)
- Adafruit_NeoPixel (1.11.0+)
- PubSubClient (2.8.0+) ← CHỈ cần cho MQTT
```

### Phần cứng:
- ESP32-S3-EYE
- PIR sensor trên GPIO14
- WS2812 LED trên GPIO48

---

## 🚀 Cách Upload

### Bước 1: Cài thư viện PubSubClient

Trong Arduino IDE:
1. `Sketch` → `Include Library` → `Manage Libraries...`
2. Tìm **"PubSubClient"** by Nick O'Leary
3. Install version **2.8.0** hoặc mới hơn

### Bước 2: Cấu hình config.h

```cpp
// ===== WIFI & SERVER CONFIGURATION =====
#define WIFI_SSID "TEN_WIFI_CUA_BAN"          // ← SỬA TÊN WIFI
#define WIFI_PASSWORD "MAT_KHAU_WIFI"         // ← SỬA MẬT KHẨU

// Keep IP 192.168.77.24 or change to your server IP
#define SERVER_BASE_URL "http://192.168.77.24:3000/api"

// Your account credentials
#define USERNAME "MinhKhue123"
#define USER_PASSWORD "123456"

// ===== MQTT CONFIGURATION =====
#define USE_MQTT true                         // ← true = MQTT, false = HTTP only
#define MQTT_BROKER "192.168.77.24"          // ← Your server IP
#define MQTT_PORT 1883
#define MQTT_CLIENT_ID "ESP32-CAM-001"       // ← Unique ID for each device
```

### Bước 3: Chọn file để upload

#### Option A: MQTT Mode (Khuyên dùng!)
1. Đổi tên **main_mqtt.ino** → **main.ino**
2. Hoặc copy nội dung từ main_mqtt.ino vào main.ino

#### Option B: HTTP Only Mode
- Giữ nguyên **main.ino** hiện tại
- Set `#define USE_MQTT false` trong config.h

### Bước 4: Upload lên ESP32

1. Connect ESP32 via USB
2. Chọn board: **ESP32S3 Dev Module**
3. Chọn port: COM port của ESP32
4. Click **Upload** (Ctrl+U)

### Bước 5: Test

1. Mở Serial Monitor (115200 baud)
2. Bật PIR sensor (hoặc reset ESP32)
3. Xem log:

```
=================================
🚀 ESP32 Motion Camera Starting
   MODE: MQTT + HTTP Fallback
=================================
🏃 WAKE: Motion detected!
[1/6] Connecting to WiFi...
✅ Connected! IP: 192.168.77.41
[2/6] Authenticating...
✅ Authenticated!
[3/6] Connecting to MQTT...
🔌 Connecting to MQTT broker...
Broker: 192.168.77.24:1883
✅ MQTT connected!
[4/6] Initializing camera...
✅ Camera ready!
[5/6] Capturing photo...
✅ Captured! Size: 185420 bytes
[6/6] Uploading image...
📡 Attempting MQTT publish...
📤 Publishing image (185420 bytes) to MQTT...
✅ Image published to MQTT!
✅ MQTT upload successful!
=================================
📊 SUMMARY
=================================
Status: SUCCESS ✅
Upload method: MQTT
Wake reason: Motion
=================================
```

---

## 🎨 LED Indicators

| Color | Meaning |
|-------|---------|
| 🟢 Green | Success (WiFi, Auth, Upload) |
| 🔴 Red | Error (WiFi fail, Camera fail, Upload fail) |
| 🔵 Blue | Motion detected wake |
| ⚪ White | Power-on wake, Camera flash |
| 🟡 Yellow | Warning (MQTT fail, fallback to HTTP) |

---

## 🐛 Troubleshooting

### Lỗi compile: "PubSubClient.h: No such file"
**Giải pháp:** Cài thư viện PubSubClient từ Library Manager

### MQTT connection failed (rc=-2)
**Giải pháp:** 
- Kiểm tra MQTT broker đang chạy: `docker ps`
- Kiểm tra IP đúng không: `ipconfig`
- Test MQTT: `docker exec -it esp32-mosquitto mosquitto_sub -t "#" -v`

### MQTT publish failed
**Giải pháp:**
- Image quá lớn → Giảm quality trong config.h
- MQTT buffer nhỏ → Đã set 256KB trong mqtt_manager.cpp
- Hệ thống tự động fallback HTTP

### WiFi connection timeout
**Giải pháp:**
- Kiểm tra SSID và password
- Kiểm tra ESP32 gần router
- Tăng `WIFI_TIMEOUT_MS` trong config.h

### Camera init failed
**Giải pháp:**
- Reset ESP32 (nút EN)
- Kiểm tra PSRAM: Phải enable trong Arduino IDE
- Board setting: Tools → PSRAM → "OPI PSRAM"

---

## 📊 So sánh HTTP vs MQTT

| Feature | HTTP | MQTT |
|---------|------|------|
| Upload speed | ~2-3s | ~1-2s |
| Server load | Cao | Thấp |
| Reliability | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| Real-time | Không | Có |
| Battery use | Trung bình | Tốt hơn |
| Complexity | Đơn giản | Trung bình |
| Fallback | Không | ✅ Auto HTTP |

**Khuyến nghị:** Dùng **MQTT mode** với HTTP fallback - best of both worlds! 🎯

---

## 🔍 Monitoring MQTT

### Xem tất cả messages:
```powershell
docker exec -it esp32-mosquitto mosquitto_sub -t "#" -v
```

### Xem chỉ images:
```powershell
docker exec -it esp32-mosquitto mosquitto_sub -t "esp32/camera/image" -v
```

### Xem status:
```powershell
docker exec -it esp32-mosquitto mosquitto_sub -t "esp32/camera/status" -v
```

### Gửi command test:
```powershell
docker exec -it esp32-mosquitto mosquitto_pub -t "esp32/camera/command" -m "capture"
```

---

## 📈 Optimization Tips

### Giảm image size:
```cpp
// In config.h
#define JPEG_QUALITY_HIGH   15  // Increase number = lower quality = smaller size
```

### Faster upload:
```cpp
// In config.h
#define USE_MQTT true  // MQTT nhanh hơn HTTP
```

### Battery optimization:
```cpp
// Disable LED to save power
// In main.ino - comment out all ledMgr calls
```

---

## ✅ Checklist

- [ ] PubSubClient library installed
- [ ] WIFI_SSID & WIFI_PASSWORD configured
- [ ] MQTT_BROKER IP correct (your server IP)
- [ ] Docker MQTT running (`docker ps` shows esp32-mosquitto)
- [ ] Backend connected to MQTT (check backend logs)
- [ ] ESP32 uploaded successfully
- [ ] Serial Monitor shows "MQTT connected!"
- [ ] Image appears in frontend gallery

---

## 🎉 Success!

Khi thấy log:
```
✅ MQTT connected!
✅ Image published to MQTT!
Status: SUCCESS ✅
Upload method: MQTT
```

Bạn đã setup thành công! 🚀

Check frontend: http://localhost:5173/gallery
