# ESP32 MJPEG Stream Server - Setup Guide

## ✅ Changes Made:

### 1. Backend (.env)
- Updated `ESP32_STREAM_URL=http://192.168.77.41:81/stream`

### 2. ESP32 Code
Added new files:
- `stream_server.h` - Stream server interface
- `stream_server.cpp` - MJPEG streaming implementation

Modified files:
- `config.h` - Added stream server configuration
- `main.ino` - Integrated stream server

### 3. Configuration (config.h)
```cpp
#define ENABLE_STREAM_SERVER true    // Enable/disable streaming
#define STREAM_SERVER_PORT 81         // HTTP port
#define STREAM_FPS 15                 // Frames per second
```

## 🔧 How to Upload to ESP32:

### Step 1: Open Arduino IDE
1. Open Arduino IDE
2. File → Open → Navigate to:
   `E:\A_NAM_4_KI_1\IOT\CuoiKy\ESP32-Camera-Backend\examples\modular\main\main.ino`

### Step 2: Verify Board Settings
- Board: **ESP32-S3-DevKitC-1**
- Flash Mode: **QIO 80MHz**
- Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**
- PSRAM: **OPI PSRAM**
- Upload Speed: **921600**
- Port: Select your COM port

### Step 3: Compile & Upload
1. Click **Verify** (✓) to compile
2. If no errors, click **Upload** (→)
3. Wait for "Done uploading"

### Step 4: Open Serial Monitor
- Baud rate: **115200**
- Watch for:
  ```
  ✅ Stream server running!
     Stream URL: http://192.168.77.41:81/stream
  ```

## 📊 Expected Behavior:

### Normal Mode (ENABLE_STREAM_SERVER = false):
- ESP32 wakes on motion
- Captures image
- Uploads via HTTP
- Goes back to deep sleep
- Ultra low power consumption

### Stream Mode (ENABLE_STREAM_SERVER = true):
- ESP32 stays awake
- Stream server runs on port 81
- Motion trigger still uploads images
- **Higher power consumption** (~200mA continuous)
- Access stream at: `http://ESP32_IP:81/stream`

## 🎥 Stream Server Features:

- **MJPEG streaming** (multipart/x-mixed-replace)
- **Configurable FPS** (default 15 FPS)
- **Multiple clients** support (up to 3 concurrent)
- **Auto frame rate control**
- **Graceful client disconnect handling**

## ⚡ Performance Notes:

### With Stream Server Enabled:
- Power: ~200-300mA (can't use battery long-term)
- RAM usage: ~150KB for stream buffer
- WiFi: Must stay connected
- Deep sleep: **DISABLED**

### Recommendations:
- Use **USB power** or **5V adapter** when streaming
- For battery operation, set `ENABLE_STREAM_SERVER false`
- Stream quality controlled by config.h settings:
  ```cpp
  #define FRAME_SIZE_HIGH FRAMESIZE_SXGA  // 1280x1024
  #define JPEG_QUALITY_HIGH 15             // Lower = better quality
  ```

## 🔄 Backend Restart Required:

After ESP32 is ready, restart backend:
```powershell
cd E:\A_NAM_4_KI_1\IOT\CuoiKy\ESP32-Camera-Backend
docker-compose restart
```

Then check backend logs:
```powershell
docker logs esp32_backend --tail 20 -f
```

## 🌐 Access Points:

After successful setup:

1. **Frontend Live Stream**: http://localhost:5174/live
2. **Backend Proxy**: http://localhost:3000/api/live
3. **Direct ESP32 Stream**: http://192.168.77.41:81/stream

## 🐛 Troubleshooting:

### "Stream server failed to start"
- Check port 81 not in use
- Verify PSRAM enabled in board settings
- Ensure enough heap memory

### "Stream unavailable"
- Verify ESP32 IP: `192.168.77.41`
- Check WiFi connection on ESP32
- Test direct URL: http://192.168.77.41:81/stream
- Check Serial Monitor for errors

### High latency / Lag
- Reduce FPS: `#define STREAM_FPS 10`
- Lower quality: `#define JPEG_QUALITY_HIGH 18`
- Check WiFi signal strength

### Camera freezes
- Reset ESP32
- Check camera power supply (needs 5V/2A minimum)
- Verify PSRAM configuration

## 📝 Next Steps:

1. ✅ Upload code to ESP32
2. ✅ Wait for "Stream server running!" message
3. ✅ Note the IP address from Serial Monitor
4. ✅ Test direct stream: http://ESP32_IP:81/stream
5. ✅ Restart backend Docker
6. ✅ Access frontend: http://localhost:5174/live

## ⚠️ Important Notes:

- **Stream mode = No deep sleep** = Higher power consumption
- Recommended for **AC powered** setups only
- For battery operation, disable stream server
- ESP32 IP must match in `.env` and actually be correct
