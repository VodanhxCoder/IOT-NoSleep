# 🌐 Auto IP Discovery - No More Hardcoded IPs!

## 🎯 Vấn đề: IP thay đổi liên tục

Mỗi lần:
- ❌ Kết nối WiFi khác
- ❌ Router restart
- ❌ DHCP renew
→ Phải sửa code và nạp lại ESP32!

---

## ✅ Giải pháp: 3 cách

### **Giải pháp 1: mDNS (Khuyên dùng!)** ⭐

ESP32 tự động tìm server qua **tên hostname** thay vì IP.

#### Cách hoạt động:
```
ESP32 hỏi: "esp32-server.local ở đâu?"
Router trả lời: "Ở 192.168.2.22"
ESP32 connect: http://192.168.2.22:3000
```

IP thay đổi? Không sao! ESP32 tự động tìm lại.

---

## 🚀 Setup mDNS

### Bước 1: Windows Setup (Server)

#### Option A: Bonjour Service (Khuyên dùng - Nhẹ nhất)
```powershell
# Download Bonjour Print Services
# Link: https://support.apple.com/kb/DL999

# Hoặc cài iTunes (có Bonjour bundled)
# Link: https://www.apple.com/itunes/download/

# Sau khi cài, verify:
Get-Service -Name "Bonjour Service"
# Status: Running ✅
```

#### Option B: Use Docker Container mDNS
Backend đã có bonjour package! Chỉ cần enable:

**File:** `app.js` (Backend đã có sẵn!)
```javascript
const bonjour = require('bonjour')();

// Advertise service
bonjour.publish({
  name: 'esp32-server',
  type: 'http',
  port: 3000
});

console.log('📡 mDNS: Broadcasting as esp32-server.local');
```

### Bước 2: Test mDNS từ Windows

```powershell
# Install dns-sd tool (nếu chưa có Bonjour)
# Or test with ping:
ping esp32-server.local

# Should resolve to your IP:
# Pinging esp32-server.local [192.168.2.22] with 32 bytes...
```

### Bước 3: ESP32 Config (Đã update!)

**File:** `config.h`
```cpp
#define USE_MDNS true
#define SERVER_HOSTNAME "esp32-server"
#define SERVER_BASE_URL "http://esp32-server.local:3000/api"

// Fallback nếu mDNS fail:
#define SERVER_FALLBACK_URL "http://192.168.2.22:3000/api"
```

**File:** `main.ino` - Add mDNS init:
```cpp
#include "mdns_helper.h"

void setup() {
    // ... after WiFi connect ...
    
    // Get dynamic server URL
    String serverURL = MDNSHelper::getServerURL();
    Serial.println("Server URL: " + serverURL);
    
    String mqttBroker = MDNSHelper::getMQTTBroker();
    Serial.println("MQTT Broker: " + mqttBroker);
}
```

---

## 🔧 Giải pháp 2: Static IP (Router)

### Set Static IP trong Router:

1. **Login to Router** (thường `192.168.1.1` hoặc `192.168.2.1`)
2. **DHCP Settings** → **DHCP Reservation**
3. Tìm máy của bạn (MAC address)
4. Set IP cố định: `192.168.2.100` (example)
5. Save & Reboot

**Ưu điểm:**
- ✅ IP không đổi
- ✅ Không cần code thêm
- ✅ Reliable nhất

**Nhược điểm:**
- ❌ Phải config mỗi router khác nhau
- ❌ Cần quyền admin router

---

## 🔧 Giải pháp 3: Static IP (Windows)

### Set IP tĩnh trên Windows:

```powershell
# Open Network Settings
ncpa.cpl

# Right-click WiFi/Ethernet → Properties
# Internet Protocol Version 4 (TCP/IPv4) → Properties
# Select "Use the following IP address"

IP address:      192.168.2.100
Subnet mask:     255.255.255.0
Default gateway: 192.168.2.1
DNS servers:     8.8.8.8, 8.8.4.4
```

**Ưu điểm:**
- ✅ Không cần router config
- ✅ IP cố định

**Nhược điểm:**
- ❌ Chỉ work cho 1 network
- ❌ Mất kết nối khi đổi WiFi

---

## 📊 So sánh các giải pháp:

| Feature | mDNS | Router Static IP | Windows Static IP |
|---------|------|------------------|-------------------|
| Auto-discovery | ✅ Yes | ❌ No | ❌ No |
| Multi-network | ✅ Yes | ❌ No | ❌ No |
| Easy setup | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ |
| Reliability | ⭐⭐⭐⭐ | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| No router access | ✅ Yes | ❌ No | ✅ Yes |
| Portable | ✅ Yes | ❌ No | ❌ No |

**Khuyến nghị:** Dùng **mDNS** + có fallback IP! 🎯

---

## 🧪 Test mDNS

### Test 1: From Windows
```powershell
ping esp32-server.local
# Should resolve to your IP
```

### Test 2: From ESP32
```cpp
// Serial Monitor should show:
🔍 Resolving hostname: esp32-server.local
✅ Resolved to: 192.168.2.22
Server URL: http://192.168.2.22:3000/api
```

### Test 3: Change Network
```
1. Disconnect WiFi
2. Connect to different WiFi
3. ESP32 auto-discovers new IP!
```

---

## 🐛 Troubleshooting

### mDNS not working?

**Check 1: Bonjour Service running?**
```powershell
Get-Service -Name "Bonjour Service"
# If not running: Start-Service "Bonjour Service"
```

**Check 2: Windows Firewall blocking?**
```powershell
# Allow mDNS port 5353
New-NetFirewallRule -DisplayName "mDNS" -Direction Inbound -Protocol UDP -LocalPort 5353 -Action Allow
```

**Check 3: Backend advertising?**
```javascript
// app.js should have:
bonjour.publish({ name: 'esp32-server', type: 'http', port: 3000 });
```

**Check 4: ESP32 mDNS library?**
```cpp
#include <ESPmDNS.h>  // Should be in ESP32 core
```

### Fallback working?

If mDNS fails, ESP32 auto-uses fallback IP:
```cpp
⚠️ mDNS resolution failed, using fallback IP
Server URL: http://192.168.2.22:3000/api
```

---

## 💡 Best Practice: Hybrid Approach

```cpp
// config.h
#define USE_MDNS true
#define SERVER_HOSTNAME "esp32-server"
#define SERVER_FALLBACK_URL "http://192.168.2.22:3000/api"  // Backup

// ESP32 will:
// 1. Try mDNS first (esp32-server.local)
// 2. If fails, use fallback IP
// 3. Still works even if mDNS down!
```

**Best of both worlds!** ✅

---

## 📝 Quick Setup Checklist

### Option A: mDNS (Recommended)
- [ ] Install Bonjour Service (or iTunes)
- [ ] Verify: `Get-Service "Bonjour Service"`
- [ ] Backend broadcasts mDNS (app.js has bonjour)
- [ ] ESP32 config has USE_MDNS=true
- [ ] Test: `ping esp32-server.local`
- [ ] Upload ESP32 code
- [ ] Verify Serial: "✅ Resolved to: ..."

### Option B: Router Static IP
- [ ] Login to router
- [ ] Find DHCP Reservation settings
- [ ] Reserve IP for your MAC address
- [ ] Reboot router
- [ ] Update config.h with static IP
- [ ] Upload ESP32 code

### Option C: Windows Static IP
- [ ] Open ncpa.cpl
- [ ] Set static IP on WiFi/Ethernet
- [ ] Use that IP in config.h
- [ ] Upload ESP32 code

---

## 🎉 Result

### Before (Hardcoded IP):
```
IP changes → Edit code → Re-upload ESP32 → Test
❌ Time consuming!
```

### After (mDNS):
```
IP changes → ESP32 auto-discovers → Works!
✅ Zero configuration!
```

---

**Status:** 🟢 Config updated with mDNS support!

**Next:** 
1. Setup Bonjour Service on Windows
2. Upload ESP32 code
3. Test auto-discovery! 🚀
