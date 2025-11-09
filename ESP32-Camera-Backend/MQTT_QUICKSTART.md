# 🚀 Quick Start - Full MQTT Setup

## Option 1: Docker (Recommended - Easiest!)

### ✅ Prerequisites
- Install [Docker Desktop](https://www.docker.com/products/docker-desktop/)
- Start Docker Desktop

### 🏃 Run
```powershell
# Double-click this file or run:
start-mqtt.bat
```

**That's it!** MQTT broker + Backend will start automatically.

---

## Option 2: Without Docker (Manual)

### ✅ Prerequisites
- Node.js installed
- MQTT Broker running (see options below)

### 🔧 MQTT Broker Options

#### A. Use Public Broker (No install)
Edit `.env`:
```env
MQTT_BROKER=mqtt://broker.hivemq.com:1883
```

#### B. Docker MQTT Only
```powershell
docker run -d --name mosquitto -p 1883:1883 -p 9001:9001 eclipse-mosquitto
```

#### C. Install Mosquitto on Windows
```powershell
# Download from: https://mosquitto.org/download/
# Or use Scoop:
scoop install mosquitto

# Start:
mosquitto -v
```

### 🏃 Run Backend
```powershell
start-backend-mqtt.bat
```

---

## 📱 ESP32 Configuration

After starting services, update ESP32 `config.h`:

```cpp
// Get your computer IP
// Run: ipconfig | findstr IPv4

#define MQTT_BROKER "192.168.77.24"  // YOUR_IP_HERE
#define MQTT_PORT 1883
#define MQTT_TOPIC_IMAGE "esp32/camera/image"
#define MQTT_TOPIC_STATUS "esp32/camera/status"
```

---

## ✅ Test MQTT Connection

### From Windows:
```powershell
# Subscribe to all topics
docker exec -it esp32-mosquitto mosquitto_sub -t "#" -v

# Or if local mosquitto:
mosquitto_sub -t "#" -v
```

### From Backend Logs:
You should see:
```
🔌 Connecting to MQTT broker: mqtt://localhost:1883
✅ MQTT connected!
📬 Subscribed to: esp32/camera/image
```

---

## 🌐 Access Services

- **Backend API**: http://localhost:3000
- **Frontend**: http://localhost:5173
- **MQTT Broker**: mqtt://localhost:1883
- **MQTT WebSocket**: ws://localhost:9001

---

## 🐛 Troubleshooting

### Docker not working?
```powershell
# Start Docker Desktop manually
# Check status:
docker ps

# View logs:
docker-compose -f docker-compose.mqtt.yml logs -f
```

### MQTT connection refused?
```powershell
# Check if broker is running:
netstat -ano | findstr :1883

# Test connection:
mosquitto_pub -h localhost -t "test" -m "hello"
```

### Backend can't connect to MQTT?
Check `.env` file:
```env
MQTT_BROKER=mqtt://localhost:1883  # For Docker: mqtt://mosquitto:1883
```

---

## 📚 Architecture

```
ESP32 Camera
    |
    | MQTT Publish
    | mqtt://YOUR_IP:1883
    | Topic: esp32/camera/image
    | Payload: Binary JPEG
    |
    v
MQTT Broker (Mosquitto)
    |
    | Subscribe
    |
    v
Backend Server (Node.js)
    | - Receive image
    | - Detect person
    | - Save to MongoDB
    | - Send notifications
    | - Emit to Socket.IO
    |
    v
Frontend (React)
    | - Real-time updates
    | - View images
    | - Gallery
```

---

## 🎯 Next Steps

1. ✅ Start MQTT broker + Backend
2. ✅ Update ESP32 with your IP
3. ✅ Upload ESP32 code
4. ✅ Trigger PIR sensor
5. ✅ Watch images appear in frontend!

---

## 💡 Production Tips

### Enable MQTT Authentication:
```powershell
docker exec -it esp32-mosquitto mosquitto_passwd -c /mosquitto/config/passwd esp32user
```

Update `.env`:
```env
MQTT_USERNAME=esp32user
MQTT_PASSWORD=your-secure-password
```

### Enable TLS/SSL:
See `mosquitto.conf` for SSL configuration.

### Monitor MQTT Traffic:
```powershell
# Install MQTT Explorer: http://mqtt-explorer.com/
# Or use Docker:
docker run -d -p 4000:4000 smeagolworms4/mqtt-explorer
```
