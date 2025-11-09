# 🔧 Quick IP Update Guide

## 🚨 Khi IP thay đổi - Chỉ 3 bước!

### Bước 1: Lấy IP mới
```powershell
ipconfig | findstr "IPv4"
```
Tìm IP WiFi/Ethernet (thường `192.168.x.x`)

### Bước 2: Update config.h
**File:** `examples/modular/main/config.h`

Sửa **2 dòng** này:
```cpp
#define SERVER_IP "192.168.2.22"           // ← SỬA ĐÂY
#define SERVER_BASE_URL "http://192.168.2.22:3000/api"  // ← VÀ ĐÂY
#define MQTT_BROKER "192.168.2.22"         // ← VÀ ĐÂY
```

Thay `192.168.2.22` bằng IP mới của bạn.

### Bước 3: Upload lại ESP32
- Click **Upload** trong Arduino IDE
- Done! ✅

---

## 💡 Tại sao mDNS không work?

### Lý do:
1. ❌ ESP32 mDNS library không stable
2. ❌ Router phải support mDNS/Bonjour
3. ❌ Windows cần Bonjour Service
4. ❌ Thêm dependency phức tạp

### Giải pháp đơn giản nhất:
✅ **Dùng IP trực tiếp** - Always works!

---

## 🎯 Best Practice: Static IP

### Option A: Router DHCP Reservation (Khuyên dùng!)
1. Login router (usually `192.168.1.1` or `192.168.2.1`)
2. DHCP Settings → DHCP Reservation
3. Find your PC (by MAC address)
4. Reserve IP: `192.168.2.100` (example)
5. **IP không đổi nữa!** ✅

### Option B: Windows Static IP
1. Press `Win + R` → Type `ncpa.cpl`
2. Right-click WiFi/Ethernet → Properties
3. IPv4 → Properties → Use following IP:
   ```
   IP:      192.168.2.100
   Subnet:  255.255.255.0
   Gateway: 192.168.2.1
   DNS:     8.8.8.8
   ```
4. **IP cố định!** ✅

---

## 📝 Current Config

```cpp
// File: config.h
#define SERVER_IP "192.168.2.22"
#define SERVER_BASE_URL "http://192.168.2.22:3000/api"
#define MQTT_BROKER "192.168.2.22"
```

**ESP32 IP:** `192.168.77.41` (Auto from DHCP)

---

## 🔍 Quick Test

### Test từ ESP32 network:
```powershell
# Ping từ PC
ping 192.168.2.22

# Test API
curl http://192.168.2.22:3000/health
```

### Expected Serial Output:
```
[1/6] Connecting to WiFi...
✅ Connected! IP: 192.168.77.41

[2/6] Authenticating...
POST http://192.168.2.22:3000/api/auth/login
✅ Authenticated!

[3/6] Connecting to MQTT...
Broker: 192.168.2.22:1883
✅ MQTT connected!
```

---

## 🐛 Troubleshooting

### "Connection refused" error?
```
❌ HTTP Error: connection refused
```

**Kiểm tra:**
1. Backend có chạy không? `docker ps`
2. IP đúng chưa? `ipconfig`
3. Firewall block? Windows Defender
4. Cùng WiFi không? PC và ESP32

### ESP32 không kết nối WiFi?
```
❌ WiFi connection failed
```

**Kiểm tra:**
1. `WIFI_SSID` đúng chưa?
2. `WIFI_PASSWORD` đúng chưa?
3. ESP32 gần router?

---

## ✅ Checklist

Khi IP thay đổi:

- [ ] Get new IP: `ipconfig | findstr IPv4`
- [ ] Update `SERVER_IP` in config.h
- [ ] Update `SERVER_BASE_URL` in config.h
- [ ] Update `MQTT_BROKER` in config.h
- [ ] Upload code to ESP32
- [ ] Test: `ping YOUR_IP`
- [ ] Monitor Serial: Should connect OK

---

## 📋 IP Change Template

Save this for quick updates:

```cpp
// ===== UPDATE THESE 3 LINES WHEN IP CHANGES =====
#define SERVER_IP "___.___.___.___"        // Your PC IP
#define SERVER_BASE_URL "http://___.___.___.___.___:3000/api"
#define MQTT_BROKER "___.___.___.___.___"  // Same as SERVER_IP
```

---

**Pro tip:** Set Static IP once → Never change again! 🎯
