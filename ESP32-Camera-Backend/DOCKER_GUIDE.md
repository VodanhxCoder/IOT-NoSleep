# 🐳 Docker Deployment Guide

## 🚀 Quick Start

### 1. Install Docker Desktop (Windows)

Download from: https://www.docker.com/products/docker-desktop/

### 2. Create `.env` file

```env
# MongoDB (use your existing connection string)
MONGODB_URI=mongodb+srv://username:password@cluster.mongodb.net/esp32_security

# JWT Secret
JWT_SECRET=your-super-secret-jwt-key-here

# MQTT (optional - will use Docker mosquitto)
MQTT_USERNAME=
MQTT_PASSWORD=

# Email (optional)
GMAIL_USER=your-email@gmail.com
GMAIL_PASS=your-app-password

# Telegram (optional)
TELEGRAM_BOT_TOKEN=your-bot-token
```

### 3. Start Everything

```powershell
# Start MQTT broker + Backend
docker-compose -f docker-compose.mqtt.yml up -d

# View logs
docker-compose -f docker-compose.mqtt.yml logs -f

# Stop everything
docker-compose -f docker-compose.mqtt.yml down
```

## 📊 Check Status

```powershell
# List running containers
docker ps

# Check backend logs
docker logs esp32-backend -f

# Check MQTT logs
docker logs esp32-mosquitto -f

# Test MQTT connection
docker exec -it esp32-mosquitto mosquitto_pub -t "test" -m "Hello"
docker exec -it esp32-mosquitto mosquitto_sub -t "test"
```

## 🌐 Access Services

- **Backend API**: http://localhost:3000
- **MQTT Broker**: mqtt://localhost:1883
- **MQTT WebSocket**: ws://localhost:9001

## 🔧 MQTT Configuration

### Get Your Computer IP

```powershell
ipconfig | findstr IPv4
```

### Update ESP32 Config

In `config.h`:
```cpp
#define MQTT_BROKER "192.168.77.24"  // Your computer IP
#define MQTT_PORT 1883
```

## 🐛 Troubleshooting

### MQTT Connection Failed

```powershell
# Check if mosquitto is running
docker ps | findstr mosquitto

# Check MQTT logs
docker logs esp32-mosquitto

# Test from host
docker run -it --rm --network esp32-camera-backend_esp32-network eclipse-mosquitto mosquitto_sub -h mosquitto -t "#" -v
```

### Backend Can't Connect to MQTT

```powershell
# Check network
docker network inspect esp32-camera-backend_esp32-network

# Restart backend
docker-compose -f docker-compose.mqtt.yml restart backend
```

### Port Already in Use

```powershell
# Check what's using port 1883
netstat -ano | findstr :1883

# Kill process
taskkill /PID <PID> /F

# Or change port in docker-compose.mqtt.yml
```

## 📦 Update Code

After changing backend code:

```powershell
# Rebuild and restart
docker-compose -f docker-compose.mqtt.yml up -d --build

# Or rebuild specific service
docker-compose -f docker-compose.mqtt.yml build backend
docker-compose -f docker-compose.mqtt.yml up -d backend
```

## 🔐 Production Setup

For production, enable MQTT authentication:

1. **Create password file:**
```powershell
docker exec -it esp32-mosquitto mosquitto_passwd -c /mosquitto/config/passwd esp32user
```

2. **Update mosquitto.conf:**
```conf
allow_anonymous false
password_file /mosquitto/config/passwd
```

3. **Restart mosquitto:**
```powershell
docker-compose -f docker-compose.mqtt.yml restart mosquitto
```

4. **Update .env:**
```env
MQTT_USERNAME=esp32user
MQTT_PASSWORD=your-password
```

## 📝 Useful Commands

```powershell
# View all logs
docker-compose -f docker-compose.mqtt.yml logs

# Follow backend logs
docker-compose -f docker-compose.mqtt.yml logs -f backend

# Stop all services
docker-compose -f docker-compose.mqtt.yml stop

# Remove all containers
docker-compose -f docker-compose.mqtt.yml down

# Remove all containers + volumes
docker-compose -f docker-compose.mqtt.yml down -v

# Restart single service
docker-compose -f docker-compose.mqtt.yml restart backend

# Execute command in container
docker exec -it esp32-backend sh
docker exec -it esp32-mosquitto sh
```

## 🚀 Deploy to Cloud

### Azure Container Instances

```powershell
# Login
az login

# Create resource group
az group create --name esp32-camera --location eastus

# Deploy
az container create --resource-group esp32-camera --file docker-compose.mqtt.yml
```

### AWS ECS

Use AWS ECS CLI or convert docker-compose to ECS task definition.

### Google Cloud Run

```powershell
# Build and push
docker build -t gcr.io/PROJECT_ID/esp32-backend .
docker push gcr.io/PROJECT_ID/esp32-backend

# Deploy
gcloud run deploy esp32-backend --image gcr.io/PROJECT_ID/esp32-backend
```

## 💡 Tips

- Use **Docker volumes** for persistent data (`./uploads`, `./logs`)
- Check **Docker Desktop** dashboard for easy management
- Use **Portainer** for web-based Docker management
- Monitor logs with **Dozzle**: `docker run -d -p 8080:8080 -v /var/run/docker.sock:/var/run/docker.sock amir20/dozzle`
