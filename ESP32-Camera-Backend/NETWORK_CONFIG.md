# 🌐 Network Configuration Updated

**Date:** November 9, 2025  
**Status:** ✅ Updated to new IP

---

## 📊 Current Network Setup

### Server (Your Computer)
```
Old IP: 192.168.77.24 ❌
New IP: 192.168.2.22  ✅

Backend:      http://192.168.2.22:3000
MQTT Broker:  mqtt://192.168.2.22:1883
Frontend:     http://localhost:5173
```

### ESP32 Configuration
**File:** `examples/modular/main/config.h`
```cpp
#define SERVER_BASE_URL "http://192.168.2.22:3000/api"  ✅
#define MQTT_BROKER "192.168.2.22"                      ✅
```

---

## ✅ What Was Updated

### 1. ESP32 Config (config.h)
```diff
- #define SERVER_BASE_URL "http://192.168.77.24:3000/api"
+ #define SERVER_BASE_URL "http://192.168.2.22:3000/api"

- #define MQTT_BROKER "192.168.77.24"
+ #define MQTT_BROKER "192.168.2.22"
```

### 2. Backend (.env)
```
MQTT_BROKER=mqtt://localhost:1883  ← No change needed!
```
Backend connects to MQTT via Docker internal network (localhost)

---

## 🔍 Network Interfaces Detected

```
172.22.0.1      - Docker bridge
192.168.137.1   - Virtual adapter
192.168.184.1   - Virtual adapter
192.168.106.1   - Virtual adapter
192.168.2.22    - WiFi/Ethernet (ACTIVE) ✅
```

---

## 🚀 Services Status

```
✅ Backend:      0.0.0.0:3000 (LISTENING)
✅ MQTT Broker:  0.0.0.0:1883 (LISTENING)
✅ WebSocket:    0.0.0.0:9001 (LISTENING)
```

**Listening on 0.0.0.0 = Accessible from any network!** ✅

---

## 📱 ESP32 Upload Steps

### 1. Verify Config
Check `config.h`:
```cpp
#define WIFI_SSID "..."              // ← Update your WiFi name
#define WIFI_PASSWORD "20041610"
#define SERVER_BASE_URL "http://192.168.2.22:3000/api"  ✅
#define MQTT_BROKER "192.168.2.22"   ✅
```

### 2. Upload to ESP32
- Click Upload in Arduino IDE
- Wait for completion

### 3. Monitor Serial
Open Serial Monitor (115200):
```
[1/6] Connecting to WiFi...
✅ Connected! IP: 192.168.2.xx

[3/6] Connecting to MQTT...
Broker: 192.168.2.22:1883          ✅
✅ MQTT connected!
```

---

## 🧪 Testing

### Test Backend API:
```powershell
curl http://192.168.2.22:3000/health
# Should return: {"status":"ok"}
```

### Test MQTT from ESP32:
```powershell
# Subscribe to all messages
docker exec -it esp32-mosquitto mosquitto_sub -t "#" -v

# Then trigger ESP32 (wave hand at PIR)
# Should see:
esp32/camera/status online
esp32/camera/image <binary data>
```

### Test from Browser:
```
Frontend: http://localhost:5173
Backend:  http://192.168.2.22:3000
Gallery:  http://localhost:5173/gallery
```

---

## 🐛 Troubleshooting

### ESP32 can't connect to WiFi
**Check:**
- WIFI_SSID correct?
- WIFI_PASSWORD correct?
- ESP32 near router?

### ESP32 can't reach server (192.168.2.22)
**Check:**
```powershell
# Ping from another device on same WiFi
ping 192.168.2.22

# Should reply. If not:
# 1. Check Windows Firewall
# 2. Make sure backend running (docker ps)
```

### MQTT connection refused
**Check:**
```powershell
# MQTT broker running?
docker ps | findstr mosquitto

# Port accessible?
netstat -ano | findstr :1883
```

### IP Changed Again?
**Quick fix:**
```powershell
# 1. Get new IP
ipconfig | findstr "IPv4"

# 2. Update config.h
#define SERVER_BASE_URL "http://NEW_IP:3000/api"
#define MQTT_BROKER "NEW_IP"

# 3. Re-upload to ESP32
```

---

## 💡 Auto-Detect IP (Future Enhancement)

**Option 1: Use mDNS**
```cpp
// ESP32 can discover server via mDNS
#define SERVER_HOSTNAME "esp32-server.local"
```

**Option 2: Use Static IP on Server**
- Windows: Network Settings → Static IP
- Prevents IP changes

**Option 3: Router DHCP Reservation**
- Router admin → Reserve IP for your MAC address

---

## 📝 Quick Reference Card

Print this and keep near your desk:

```
┌─────────────────────────────────────┐
│  ESP32 CAMERA NETWORK CONFIG        │
├─────────────────────────────────────┤
│  Server IP:    192.168.2.22         │
│  Backend:      :3000                │
│  MQTT:         :1883                │
│  WebSocket:    :9001                │
│  Frontend:     localhost:5173       │
├─────────────────────────────────────┤
│  WiFi SSID:    ...                  │
│  Password:     20041610             │
│  Username:     MinhKhue123          │
│  User Pass:    123456               │
└─────────────────────────────────────┘
```

---

## ✅ Checklist

- [x] Get new IP: 192.168.2.22
- [x] Update config.h (SERVER_BASE_URL)
- [x] Update config.h (MQTT_BROKER)
- [x] Verify backend running (port 3000)
- [x] Verify MQTT running (port 1883)
- [ ] Upload code to ESP32
- [ ] Test WiFi connection
- [ ] Test MQTT publish
- [ ] Verify images in gallery

---

**Status:** 🟢 Config updated, ready to upload!

**Next:** Upload code to ESP32 and test! 🚀
