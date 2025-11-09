# MJPEG Live Stream Implementation - Complete Guide

## 📦 What Has Been Done:

### ✅ Backend (Node.js)
1. **Updated `.env`**:
   - Changed ESP32_STREAM_URL from `192.168.1.100` → `192.168.77.41`
   - Backend now points to correct ESP32 IP

2. **Existing `/api/live` endpoint** (already working):
   - Proxies MJPEG stream from ESP32
   - Handles multipart/x-mixed-replace
   - Auto-cleanup on client disconnect

3. **Docker containers restarted** to load new config

### ✅ ESP32 Firmware
**New Files Created:**
- `stream_server.h` - Stream server interface
- `stream_server.cpp` - MJPEG implementation with:
  - HTTP server on port 81
  - `/stream` endpoint
  - Frame rate control (15 FPS)
  - Multi-client support (up to 3)
  - Graceful disconnect handling

**Modified Files:**
- `config.h` - Added:
  ```cpp
  #define ENABLE_STREAM_SERVER true
  #define STREAM_SERVER_PORT 81
  #define STREAM_FPS 15
  ```

- `main.ino` - Added:
  - Stream server initialization
  - Keep-alive logic (no deep sleep when streaming)
  - WiFi reconnect in loop()

### ✅ Frontend (React)
- `LiveStream.jsx` - Already implemented, no changes needed
- Connects to backend proxy at `http://localhost:3000/api/live`

---

## 🚀 HOW TO DEPLOY:

### Step 1: Upload ESP32 Firmware ⏱️ 5 minutes

1. **Open Arduino IDE**
   
2. **Open project**:
   ```
   File → Open
   Navigate to: E:\A_NAM_4_KI_1\IOT\CuoiKy\ESP32-Camera-Backend\examples\modular\main\main.ino
   ```

3. **Verify board settings**:
   - Board: `ESP32-S3-DevKitC-1 (or ESP32S3 Dev Module)`
   - Upload Speed: `921600`
   - Flash Mode: `QIO 80MHz`
   - Partition Scheme: `Huge APP (3MB No OTA/1MB SPIFFS)`
   - PSRAM: `OPI PSRAM` (REQUIRED!)
   - Port: `Select your COM port`

4. **Compile and Upload**:
   - Click Verify (✓) → Wait for "Done compiling"
   - Click Upload (→) → Wait for "Done uploading"

5. **Open Serial Monitor** (Ctrl+Shift+M):
   - Baud: `115200`
   - Look for:
     ```
     ✅ Stream server running!
        Stream URL: http://192.168.77.41:81/stream
     ```

### Step 2: Test ESP32 Stream ⏱️ 1 minute

Open browser and test **direct stream**:
```
http://192.168.77.41:81/stream
```

**Expected**: You should see live video from ESP32 camera

**If not working**:
- Check ESP32 Serial Monitor for errors
- Verify ESP32 IP with `Serial.println(WiFi.localIP())`
- Update IP in `.env` if different

### Step 3: Backend Already Running ✅

Backend is already restarted with new config. Verify:
```powershell
docker logs esp32_backend --tail 10
```

Should show:
```
✅ MongoDB Connected
📸 Live Stream: http://localhost:3000/api/live
```

### Step 4: Test Frontend ⏱️ 1 minute

1. **Open**: http://localhost:5174/live

2. **Expected behavior**:
   - Shows "Connecting to stream..."
   - Then displays live video
   - Status shows "Connected" with green dot

**If shows "Stream Unavailable"**:
- Check ESP32 stream is working (Step 2)
- Check backend logs for proxy errors
- Verify ESP32 IP in `.env` matches actual IP

---

## 🎯 Architecture Overview:

```
┌─────────────┐
│   ESP32     │
│  Port 81    │ ←── MJPEG Stream Server
│  /stream    │     (New code added)
└──────┬──────┘
       │
       │ Stream: http://192.168.77.41:81/stream
       │
       ↓
┌──────────────────┐
│  Backend Server  │
│  Port 3000       │ ←── Stream Proxy
│  /api/live       │     (Already exists)
└────────┬─────────┘
         │
         │ Proxy to: http://localhost:3000/api/live
         │
         ↓
┌─────────────────┐
│  Frontend       │
│  Port 5174      │ ←── LiveStream.jsx
│  /live          │     (No changes needed)
└─────────────────┘
```

---

## 📊 System Status After Implementation:

### ✅ Working Components:
1. **Image Upload** - ESP32 → Backend → MongoDB (via HTTP)
2. **Gallery** - View all captured images
3. **Dashboard** - Recent images with stats
4. **Notifications** - Auto-update when new images arrive
5. **Download** - Fixed with blob download
6. **Live Stream** - Real-time MJPEG (after ESP32 upload)

### ⚡ Power Modes:

**Mode 1: Deep Sleep (ENABLE_STREAM_SERVER = false)**
- Power: ~10µA sleep, wakes on motion
- Battery life: ~6 months on 3000mAh
- Use case: Security camera, motion detection

**Mode 2: Streaming (ENABLE_STREAM_SERVER = true)** ← Current
- Power: ~200-300mA continuous
- Battery life: ~10 hours on 3000mAh
- **Requires AC power adapter**
- Use case: Live monitoring, continuous surveillance

---

## 🔧 Configuration Options:

### Adjust Stream Quality (ESP32):

Edit `config.h`:

```cpp
// Lower FPS = Less bandwidth
#define STREAM_FPS 10        // 10-30 FPS recommended

// Lower quality number = Better quality but larger size
#define JPEG_QUALITY_HIGH 12  // 10-20 recommended

// Smaller frame = Lower resolution but faster
#define FRAME_SIZE_HIGH FRAMESIZE_SVGA  // or FRAMESIZE_VGA
```

### Enable/Disable Stream:

```cpp
// To disable streaming and go back to deep sleep mode:
#define ENABLE_STREAM_SERVER false
```

---

## 🐛 Troubleshooting:

### Problem: "Stream Unavailable" in Frontend

**Check 1 - ESP32 IP**:
```
Serial Monitor → Find "IP: 192.168.77.41"
Compare with .env: ESP32_STREAM_URL
```

**Check 2 - Direct Stream**:
```
Open: http://192.168.77.41:81/stream
Should see video
```

**Check 3 - Backend Proxy**:
```powershell
curl http://localhost:3000/api/live
Should stream MJPEG data
```

### Problem: ESP32 Stream Server Won't Start

**Solution 1 - PSRAM**:
```
Arduino IDE → Tools → PSRAM → OPI PSRAM
Re-upload firmware
```

**Solution 2 - Partition**:
```
Tools → Partition Scheme → Huge APP (3MB)
Re-upload firmware
```

**Solution 3 - Port Conflict**:
```cpp
// Change port in config.h:
#define STREAM_SERVER_PORT 8081
// Update .env:
ESP32_STREAM_URL=http://192.168.77.41:8081/stream
```

### Problem: High Latency / Lag

**Reduce FPS**:
```cpp
#define STREAM_FPS 8  // Instead of 15
```

**Lower Quality**:
```cpp
#define JPEG_QUALITY_HIGH 18  // Higher number = lower quality
```

**Smaller Resolution**:
```cpp
#define FRAME_SIZE_HIGH FRAMESIZE_VGA  // 640x480
```

---

## 📝 Testing Checklist:

- [ ] ESP32 firmware uploaded successfully
- [ ] Serial Monitor shows "Stream server running!"
- [ ] Direct stream works: http://192.168.77.41:81/stream
- [ ] Backend logs show no errors
- [ ] Frontend Live page shows video
- [ ] Status shows "Connected" with green dot
- [ ] Motion detection still triggers image upload
- [ ] Gallery updates with new captures

---

## ⚠️ Important Notes:

1. **Stream mode disables deep sleep** - ESP32 stays awake continuously
2. **Requires external power** - Cannot run long on battery
3. **Motion detection still works** - Images still uploaded to backend
4. **Multiple modes available**:
   - Stream + Motion Upload (current)
   - Stream only
   - Motion only (deep sleep)

---

## 🎉 Expected Result:

After completing all steps, you should have:

✅ **Live video feed** at http://localhost:5174/live
✅ **Motion-triggered captures** still working
✅ **Real-time notifications** when motion detected
✅ **Full gallery** of all images
✅ **Dual functionality**: Live view + Security capture

---

**Ready to upload? Follow Step 1 above!** 🚀

Need help? Check the troubleshooting section or review Serial Monitor output.
