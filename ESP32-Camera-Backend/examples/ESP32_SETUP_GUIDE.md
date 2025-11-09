# 📱 ESP32 Setup Guide - Hướng Dẫn Cấu Hình ESP32

## 🎯 Tổng Quan

Code ESP32 đã được cập nhật để **TỰ ĐỘNG LOGIN** và lấy JWT token. Bạn không cần phải copy token thủ công nữa!

---

## 🔧 Cấu Hình Cần Thiết

### 1. Thông Tin WiFi
```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Tên WiFi của bạn
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Mật khẩu WiFi
```

### 2. Địa Chỉ Server
```cpp
const char* SERVER_BASE_URL = "http://192.168.1.100:3000/api";
```
**Chú ý**: Thay `192.168.1.100` bằng IP của máy tính chạy backend:
- Mở Command Prompt (Windows)
- Gõ: `ipconfig`
- Tìm "IPv4 Address" của adapter đang dùng WiFi
- Ví dụ: `192.168.1.105`

### 3. Thông Tin Đăng Nhập
```cpp
const char* USERNAME = "Minh Khue";    // Username đã đăng ký
const char* USER_PASSWORD = "123456";   // Password của bạn
```
**Quan trọng**: Đây là username/password bạn đã đăng ký trên web!

---

## 🚀 Cách Hoạt Động

### Luồng Tự Động Login

```
ESP32 Boot
    ↓
Connect WiFi
    ↓
Auto Login → POST /api/auth/login
    ↓
Nhận JWT Token (lưu vào biến jwtToken)
    ↓
Initialize Camera
    ↓
Ready! (Chờ PIR phát hiện chuyển động)
    ↓
Motion Detected → Chụp Ảnh
    ↓
Upload → POST /api/upload-image (với Bearer token)
    ↓
Backend xử lý (AI detection, notifications)
```

### Token Expiry Handling
- Nếu token hết hạn (401 error)
- ESP32 tự động login lại
- Lấy token mới
- Retry upload ảnh

---

## 📝 Các Bước Setup ESP32

### Bước 1: Cài Đặt Arduino IDE & Libraries

1. **Download Arduino IDE**: https://www.arduino.cc/en/software

2. **Cài ESP32 Board**:
   - Mở Arduino IDE
   - File → Preferences
   - Additional Boards Manager URLs: 
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Tools → Board → Boards Manager
   - Tìm "esp32" → Install

3. **Cài Libraries** (Sketch → Include Library → Manage Libraries):
   - `ArduinoJson` (by Benoit Blanchon)
   - `ESP32 Camera` (by Espressif)
   - WiFi, HTTPClient (đã có sẵn)

### Bước 2: Mở & Cấu Hình Code

1. Mở file `ESP32_EXAMPLE.cpp` trong Arduino IDE
2. Sửa các thông tin:
   ```cpp
   const char* WIFI_SSID = "Ten_WiFi_Nha_Ban";
   const char* WIFI_PASSWORD = "MatKhau_WiFi";
   const char* SERVER_BASE_URL = "http://192.168.1.XXX:3000/api";
   const char* USERNAME = "Minh Khue";
   const char* USER_PASSWORD = "123456";
   ```

### Bước 3: Chọn Board & Port

1. **Tools → Board → ESP32 Arduino → ESP32S3 Dev Module**
2. **Tools → Port → COM_X** (chọn port ESP32 đang kết nối)

### Bước 4: Upload Code

1. Kết nối ESP32 với máy tính qua USB
2. Click nút **Upload** (mũi tên →)
3. Đợi compile và upload xong

### Bước 5: Mở Serial Monitor

1. **Tools → Serial Monitor**
2. Baud rate: **115200**
3. Xem log để debug

---

## 📺 Serial Monitor Log Mẫu

```
=== ESP32-S3-EYE Security Camera ===
Connecting to WiFi........
✓ WiFi connected
IP Address: 192.168.1.150

Logging in to server...
Login URL: http://192.168.1.100:3000/api/auth/login
Request: {"username":"Minh Khue","password":"123456"}
HTTP Response code: 200
✓ Token received
✓ Logged in successfully
Token: eyJhbGciOiJIUzI1NiI...

Initializing camera...
PSRAM found - Using high quality
✓ Camera initialized

System ready! Monitoring for motion...

🚨 Motion detected!
Capturing image...
Image size: 87234 bytes
Uploading to server...
HTTP Response code: 201
Person detected! Image saved and notifications sent.
✓ Image uploaded successfully
```

---

## 🔍 Troubleshooting

### Lỗi: WiFi connection failed
- ✅ Kiểm tra SSID và password đúng chưa
- ✅ ESP32 có trong phạm vi WiFi không
- ✅ WiFi có internet không (không bắt buộc, chỉ cần local network)

### Lỗi: Login failed
- ✅ Kiểm tra username/password đã đăng ký trên web chưa
- ✅ Kiểm tra SERVER_BASE_URL có đúng IP không
- ✅ Backend server có đang chạy không
- ✅ Firewall có block port 3000 không

### Lỗi: Camera init failed
- ✅ Kiểm tra kết nối camera module
- ✅ Đảm bảo dùng đúng board ESP32-S3-EYE
- ✅ Reset ESP32 và thử lại

### Lỗi: Upload failed (HTTP 401)
- ✅ Token đã hết hạn → ESP32 sẽ tự động login lại
- ✅ Nếu vẫn lỗi, kiểm tra JWT_SECRET trong backend .env

### Lỗi: No person detected
- ✅ Bình thường! Backend trả về khi không phát hiện người
- ✅ OpenCV detection đang disabled, nên sẽ luôn return true
- ✅ Nếu muốn test, bất kỳ ảnh nào cũng sẽ được save

---

## 🔐 Bảo Mật

### Lưu Ý Quan Trọng:
1. **Không commit password lên Git**
   - Đặt credentials trong file riêng
   - Thêm vào `.gitignore`

2. **Dùng HTTPS trong production**
   - HTTP chỉ dùng local testing
   - Production nên dùng HTTPS

3. **Token có thời hạn**
   - Mặc định: 7 ngày
   - ESP32 tự động renew khi hết hạn

---

## 🎛️ Tùy Chỉnh

### Thay Đổi Thời Gian Cooldown
```cpp
const unsigned long COOLDOWN_PERIOD = 5000;  // 5 giây
```
- Tránh chụp ảnh quá nhiều
- Giảm tải cho server

### Thay Đổi Chất Lượng Ảnh
```cpp
config.frame_size = FRAMESIZE_UXGA;  // 1600x1200
config.jpeg_quality = 10;             // 0-63 (thấp = chất lượng cao)
```

Các frame sizes:
- `FRAMESIZE_UXGA` - 1600x1200 (High quality)
- `FRAMESIZE_SXGA` - 1280x1024
- `FRAMESIZE_XGA` - 1024x768
- `FRAMESIZE_SVGA` - 800x600 (Standard)
- `FRAMESIZE_VGA` - 640x480 (Low quality, nhanh)

### Pin PIR Sensor
```cpp
const int PIR_PIN = 13;  // Thay đổi tùy theo cách bạn đấu nối
```

---

## 🧪 Test Upload Thủ Công

Nếu muốn test upload ảnh mà không cần PIR:

### Cách 1: Thêm vào `loop()`
```cpp
void loop() {
  // Test upload every 10 seconds
  static unsigned long lastTest = 0;
  if (millis() - lastTest > 10000) {
    captureAndUploadImage();
    lastTest = millis();
  }
}
```

### Cách 2: Dùng Serial Command
Thêm vào `loop()`:
```cpp
void loop() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'c' || cmd == 'C') {
      Serial.println("Manual capture triggered!");
      captureAndUploadImage();
    }
  }
  
  // ... existing PIR code ...
}
```
Gõ 'c' trong Serial Monitor để chụp ảnh thủ công.

---

## 📊 API Response Codes

| Code | Meaning | Action |
|------|---------|--------|
| 200 | OK | Success |
| 201 | Created | Image saved successfully |
| 400 | Bad Request | Check request format |
| 401 | Unauthorized | Token expired → Auto re-login |
| 500 | Server Error | Backend error, check logs |

---

## 🔄 Flow Chart Chi Tiết

```
┌─────────────────┐
│   ESP32 Boot    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Connect WiFi   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐      ┌──────────────────┐
│  Login to API   │─────>│ Receive JWT Token│
│ POST /auth/login│      └──────────────────┘
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Init Camera    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Wait for PIR   │◄──────┐
└────────┬────────┘       │
         │                │
         ▼ Motion         │
┌─────────────────┐       │
│  Capture Photo  │       │
└────────┬────────┘       │
         │                │
         ▼                │
┌─────────────────┐       │
│  Upload Image   │       │
│ POST /upload    │       │
│ Bearer: token   │       │
└────────┬────────┘       │
         │                │
    ┌────┴────┐           │
    │         │           │
    ▼         ▼           │
┌─────┐   ┌──────┐        │
│ 200 │   │ 401  │        │
│ OK  │   │Expire│        │
└──┬──┘   └───┬──┘        │
   │          │           │
   │          ▼           │
   │    ┌─────────┐       │
   │    │Re-login │       │
   │    └────┬────┘       │
   │         │            │
   └─────────┴────────────┘
```

---

## ✅ Checklist Trước Khi Chạy

- [ ] Backend server đang chạy (port 3000)
- [ ] MongoDB đã kết nối
- [ ] Đã đăng ký tài khoản trên web
- [ ] Đã cấu hình WiFi SSID/Password
- [ ] Đã sửa SERVER_BASE_URL với IP đúng
- [ ] Đã cấu hình USERNAME/PASSWORD
- [ ] Đã cài đặt libraries cần thiết
- [ ] Đã chọn đúng board ESP32S3
- [ ] Camera module đã kết nối đúng
- [ ] PIR sensor đã đấu nối (hoặc test không cần PIR)

---

## 🎓 Kết Luận

Code ESP32 đã được nâng cấp để:
- ✅ **Tự động login** khi khởi động
- ✅ **Tự động renew token** khi hết hạn
- ✅ **Tự động retry** khi upload thất bại
- ✅ **Debug dễ dàng** với Serial log chi tiết

Giờ bạn chỉ cần:
1. Cấu hình WiFi + credentials
2. Upload code lên ESP32
3. Xong! ESP32 sẽ tự động login và sẵn sàng hoạt động

**Happy coding!** 🚀
