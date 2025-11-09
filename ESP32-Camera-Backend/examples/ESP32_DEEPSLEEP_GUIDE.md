# 🔋 ESP32 Motion Detection with Deep Sleep - Hướng Dẫn

## 🎯 Tính Năng Chính

Code này kết hợp **Deep Sleep** (tiết kiệm pin) với **auto-login** và **upload cloud**:

1. **Deep Sleep Mode** - ESP32 ngủ sâu, tiêu thụ chỉ ~10µA
2. **Wake on Motion** - PIR sensor đánh thức ESP32 (ext0 wake)
3. **Auto Capture** - Chụp ảnh ngay khi thức dậy
4. **Flash LED** - WS2812 nhấp nháy khi chụp
5. **Auto Upload** - Tự động login và upload lên server
6. **Token Cache** - Lưu JWT token trong RTC memory (không mất khi sleep)
7. **Back to Sleep** - Sau khi upload xong, quay lại deep sleep

---

## 📊 So Sánh 2 Phiên Bản Code

| Feature | ESP32_EXAMPLE.cpp | ESP32_MOTION_DEEPSLEEP.cpp |
|---------|-------------------|----------------------------|
| **Power Mode** | Always ON | Deep Sleep (Low Power) |
| **Power Consumption** | ~160mA | ~10µA (sleep) + ~250mA (active) |
| **Battery Life** | Vài giờ | Vài tuần/tháng |
| **PIR Detection** | Continuous polling | Wake-on-interrupt |
| **WiFi** | Always connected | Connect on demand |
| **Token Storage** | RAM (lost on reset) | RTC memory (persists) |
| **Use Case** | AC powered, test | Battery powered, production |
| **Boot Time** | Fast (~2s) | Very fast (~500ms from sleep) |
| **Cooldown** | 5s delay in code | Natural (sleep duration) |

---

## 🔧 Cấu Hình

### 1. WiFi & Server
```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_BASE_URL = "http://192.168.2.22:3000/api";  // ✅ Dùng IP của bạn
const char* USERNAME = "Minh Khue";
const char* USER_PASSWORD = "123456";
```

### 2. Hardware Pins
```cpp
#define PIR_PIN      14    // ✅ GPIO14 - RTC capable (ext0 wake)
#define WS2812_PIN   48    // LED flash
```

**Lưu ý**: GPIO14 là pin RTC-capable, có thể wake từ deep sleep!

### 3. Timing
```cpp
const uint32_t FLASH_DURATION_MS = 150;       // Flash 150ms
const uint32_t POST_UPLOAD_DELAY_MS = 2000;   // Đợi 2s trước khi sleep
const uint32_t WIFI_TIMEOUT_MS = 15000;       // WiFi timeout 15s
```

---

## 🔄 Luồng Hoạt Động

### First Boot (Lần Đầu Khởi Động)
```
Power ON
    ↓
Setup()
    ↓
Check wake reason → NORMAL BOOT
    ↓
Flash Blue LED (ready)
    ↓
Configure ext0 wake (PIR_PIN)
    ↓
Enter Deep Sleep 💤
    ↓
Wait for PIR motion...
```

### Motion Detected (Có Chuyển Động)
```
PIR Triggered (GPIO14 → HIGH)
    ↓
ESP32 Wakes Up ⏰
    ↓
Check wake reason → EXT0 (PIR)
    ↓
Flash Green LED
    ↓
Connect WiFi
    ↓
Restore Token from RTC memory
    ↓
(If no token) → Login & Save Token
    ↓
Init Camera
    ↓
Capture Image 📸
    ↓
Flash White LED (flash)
    ↓
Upload to Server 📤
    ↓
✓ Success → Flash Green
✗ Failed → Try Re-login → Retry
    ↓
Cleanup (deinit camera, disconnect WiFi)
    ↓
Delay 2s
    ↓
Configure ext0 wake again
    ↓
Enter Deep Sleep 💤
```

---

## 🔋 Power Consumption

### Deep Sleep Mode
- **Current**: ~10 µA
- **Duration**: 99% of time
- **Battery life**: Với pin 2000mAh → ~8-12 tháng (nếu 10 motion/ngày)

### Active Mode (khi có motion)
- **WiFi connect**: ~150mA x 2-5s
- **Camera capture**: ~250mA x 1-2s
- **Upload**: ~200mA x 2-5s
- **Total active time**: ~10-15s per motion

### Ví Dụ Tính Toán
```
Assumptions:
- 10 motion events per day
- 15s active per event
- 2000mAh battery

Active time per day: 10 × 15s = 150s = 2.5 minutes
Sleep time per day: 24h - 2.5min ≈ 23.96 hours

Active consumption: 250mA × (2.5/60)h = 10.4 mAh/day
Sleep consumption: 0.01mA × 23.96h = 0.24 mAh/day
Total: ~10.6 mAh/day

Battery life: 2000mAh / 10.6mAh = 188 days ≈ 6 months
```

---

## 💾 RTC Memory (Token Cache)

### Tại Sao Lưu Token Trong RTC Memory?
```cpp
RTC_DATA_ATTR char savedToken[512] = "";
```

1. **RTC memory** không bị xóa khi deep sleep
2. **Giảm số lần login** → tiết kiệm thời gian & pin
3. **Token có thể dùng lại** trong nhiều lần wake

### Khi Nào Token Bị Clear?
- Hard reset (nhấn nút reset)
- Power cycle (tắt nguồn)
- Token expired (401 error từ server)

---

## 🚨 Debug & LED Indicators

| LED Color | Meaning |
|-----------|---------|
| 🔵 Blue (300ms) | Initial boot, system ready |
| 🟢 Green (100ms) | Woke by motion |
| ⚪ White (150ms) | Camera flash |
| 🟢 Green (200ms) | Upload success |
| 🔴 Red (blink 3x) | Camera init failed |
| 🔴 Red (blink 5x) | Upload failed |
| 🔴 Red (blink 10x) | Login failed |

---

## 📝 Serial Monitor Output

### Normal Boot
```
=== ESP32-S3-EYE Motion Detection System ===
Boot count: 1
🔌 Initial boot - configuring wake on motion

💤 Configuring deep sleep...
Wake trigger: PIR motion on GPIO14 (HIGH)
Entering deep sleep NOW...
```

### Motion Detected
```
=== ESP32-S3-EYE Motion Detection System ===
Boot count: 2
🚨 Woke by PIR motion detection!
Connecting to WiFi....
✓ WiFi connected
IP: 192.168.2.50
✓ Token restored from RTC memory
Initializing camera...
PSRAM found - High quality mode
✓ Camera ready
📸 Capturing image...
Image size: 87234 bytes (1600x1200)
📤 Uploading to server...
HTTP 201
Person detected! Image saved and notifications sent.
✓ Success! Image uploaded.

💤 Configuring deep sleep...
Wake trigger: PIR motion on GPIO14 (HIGH)
Entering deep sleep NOW...
```

---

## 🔧 Advanced Configuration

### 1. Thay Đổi Image Quality
```cpp
if (psramFound()) {
  config.frame_size = FRAMESIZE_UXGA;  // 1600x1200 - High
  // Or: FRAMESIZE_SXGA (1280x1024)
  // Or: FRAMESIZE_HD (1280x720)
  config.jpeg_quality = 10;  // 0-63, lower = better
}
```

### 2. Thêm Timer Wake (Backup)
Nếu PIR không hoạt động, wake mỗi 1 giờ:
```cpp
void enterDeepSleep() {
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIR_PIN, 1);
  
  // Thêm dòng này:
  esp_sleep_enable_timer_wakeup(3600 * 1000000ULL); // 1 hour
  
  esp_deep_sleep_start();
}
```

### 3. Bật SD Card Backup
Uncomment các dòng sau trong code:
```cpp
// Ở đầu file:
#include "FS.h"
#include "SD_MMC.h"

// Trong captureAndUpload():
// Sau khi upload thành công, save to SD:
saveToSD(fb);
```

---

## 🧪 Testing

### Test 1: Initial Boot
1. Upload code
2. Mở Serial Monitor (115200 baud)
3. Xem log: "Initial boot" → "Entering deep sleep"
4. LED nhấp xanh dương

### Test 2: Motion Detection
1. Vẫy tay trước PIR sensor
2. ESP32 wake up (xem Serial log)
3. LED nhấp xanh lá → trắng → xanh lá
4. Check server: ảnh đã được upload

### Test 3: Token Cache
1. Trigger motion lần 1 → Login
2. Đợi sleep
3. Trigger motion lần 2 → Không login (dùng cached token)
4. Check log: "Token restored from RTC memory"

### Test 4: Token Expiry
1. Đợi token hết hạn (7 ngày mặc định)
2. Trigger motion
3. Upload failed (401)
4. Auto re-login
5. Retry upload thành công

---

## ⚠️ Troubleshooting

### ESP32 không wake
- ✅ Kiểm tra PIR có nguồn không (VCC = 3.3V hoặc 5V)
- ✅ PIR output có kết nối đúng GPIO14 không
- ✅ Test PIR: kết nối GPIO14 với GND/VCC xem wake không

### WiFi connection failed
- ✅ SSID/password đúng chưa
- ✅ ESP32 trong phạm vi WiFi không
- ✅ Tăng WIFI_TIMEOUT_MS lên 30000

### Upload failed
- ✅ Server có đang chạy không (http://192.168.2.22:3000/health)
- ✅ IP address có đúng không
- ✅ Check firewall blocking port 3000

### Camera init failed
- ✅ Camera module kết nối đúng không
- ✅ PSRAM có được detect không
- ✅ Giảm frame_size xuống FRAMESIZE_QVGA thử

### Battery drain quá nhanh
- ✅ Đo current trong sleep mode (phải ~10µA)
- ✅ Kiểm tra LED có tắt hết không
- ✅ Giảm jpeg_quality xuống 15-20 (ảnh nhỏ hơn, upload nhanh hơn)

---

## 📐 Hardware Wiring

### PIR Sensor
```
PIR VCC → ESP32 3.3V hoặc 5V
PIR GND → ESP32 GND
PIR OUT → ESP32 GPIO14
```

### WS2812 LED (Built-in on ESP32-S3-EYE)
```
LED → GPIO48 (already connected on board)
```

### SD Card (Optional)
```
CLK → GPIO39
CMD → GPIO38
D0  → GPIO40
```

---

## 🎓 Khi Nào Dùng Code Nào?

### Dùng **ESP32_EXAMPLE.cpp** khi:
- ✅ Có nguồn điện ổn định (USB/AC adapter)
- ✅ Đang test/development
- ✅ Cần continuous monitoring
- ✅ Không quan tâm pin

### Dùng **ESP32_MOTION_DEEPSLEEP.cpp** khi:
- ✅ Dùng pin/battery
- ✅ Production deployment
- ✅ Cần tiết kiệm điện
- ✅ Motion detection là đủ (không cần continuous)

---

## 🎯 Tối Ưu Hóa Pin

### 1. Giảm Active Time
```cpp
// Giảm chất lượng ảnh
config.frame_size = FRAMESIZE_SVGA;  // thay vì UXGA
config.jpeg_quality = 15;             // thay vì 10

// Giảm timeout
const uint32_t WIFI_TIMEOUT_MS = 10000;  // 10s thay vì 15s
```

### 2. Tăng Cooldown (Bằng PIR Delay)
- Chỉnh PIR sensor delay time (trên module PIR)
- Hoặc check PIR state trước khi capture:
```cpp
// Trong setup(), sau khi wake:
delay(500);  // Đợi PIR stabilize
if (digitalRead(PIR_PIN) == LOW) {
  // False trigger, go back to sleep
  enterDeepSleep();
}
```

### 3. Disable Unused Features
```cpp
// Tắt Bluetooth
btStop();

// Tắt WiFi sleep (khi đã connect)
WiFi.setSleep(false);  // Faster upload
```

---

## ✅ Checklist

- [ ] Đã cấu hình WIFI_SSID/PASSWORD
- [ ] Đã sửa SERVER_BASE_URL với IP đúng (192.168.2.22)
- [ ] Đã cấu hình USERNAME/PASSWORD
- [ ] PIR sensor đã kết nối GPIO14
- [ ] Đã cài thư viện Adafruit_NeoPixel
- [ ] Backend server đang chạy
- [ ] Đã test PIR hoạt động (LED built-in nháy)
- [ ] Serial Monitor 115200 baud

---

**Phiên bản Deep Sleep sẵn sàng! Battery life lên tới 6 tháng!** 🔋🚀
