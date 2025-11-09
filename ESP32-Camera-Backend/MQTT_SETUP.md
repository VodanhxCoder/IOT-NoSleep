# 📦 Install Mosquitto MQTT Broker on Windows

## Method 1: Using Chocolatey (Recommended)

```powershell
# Install Chocolatey first (if not installed)
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install Mosquitto
choco install mosquitto -y
```

## Method 2: Manual Download

1. Download from: https://mosquitto.org/download/
2. Run installer: `mosquitto-X.X.X-install-windows-x64.exe`
3. Install to: `C:\Program Files\mosquitto`

## 🚀 Start Mosquitto

### As Windows Service (Auto-start):
```powershell
# Start service
net start mosquitto

# Stop service
net stop mosquitto

# Check status
sc query mosquitto
```

### Manual Start (for testing):
```powershell
cd "C:\Program Files\mosquitto"
mosquitto.exe -v -c mosquitto.conf
```

## 🔧 Configure Mosquitto

Edit `C:\Program Files\mosquitto\mosquitto.conf`:

```conf
# Allow anonymous connections (for development)
allow_anonymous true

# Listen on all interfaces
listener 1883 0.0.0.0

# Enable websockets (optional for web clients)
listener 9001
protocol websockets

# Logging
log_dest file C:/Program Files/mosquitto/mosquitto.log
log_type all
```

## 🧪 Test MQTT Connection

### Test with mosquitto_pub/sub:

**Terminal 1 - Subscribe:**
```powershell
mosquitto_sub -h localhost -t "test/topic" -v
```

**Terminal 2 - Publish:**
```powershell
mosquitto_pub -h localhost -t "test/topic" -m "Hello MQTT!"
```

## 🐛 Troubleshooting

### Firewall Issues:
```powershell
# Add firewall rule
netsh advfirewall firewall add rule name="Mosquitto MQTT" dir=in action=allow protocol=TCP localport=1883

# Or disable firewall temporarily for testing
Set-NetFirewallProfile -Profile Domain,Public,Private -Enabled False
```

### Port Already in Use:
```powershell
# Check what's using port 1883
netstat -ano | findstr :1883

# Kill process (replace PID)
taskkill /PID <PID> /F
```

### Permission Issues:
- Run PowerShell as Administrator
- Check Windows Services permissions

## ✅ Verify Installation

After starting Mosquitto, check:

1. **Service Running:**
   ```powershell
   Get-Service mosquitto
   ```

2. **Port Listening:**
   ```powershell
   netstat -ano | findstr :1883
   ```

3. **Test Connection:**
   ```powershell
   mosquitto_sub -h localhost -t "test" -v
   ```

## 🌐 Alternative: Use Public MQTT Broker (No Install)

If you don't want to install Mosquitto, use a public broker:

### Update `.env`:
```env
# HiveMQ Public Broker
MQTT_BROKER=mqtt://broker.hivemq.com:1883

# Eclipse Public Broker
# MQTT_BROKER=mqtt://mqtt.eclipseprojects.io:1883

# Test MQTT Broker
# MQTT_BROKER=mqtt://test.mosquitto.org:1883
```

**⚠️ Warning:** Public brokers are not secure! Use only for testing.

## 📚 MQTT Topics for ESP32

Our system uses these topics:

- `esp32/camera/image` - Image uploads (Base64 or binary)
- `esp32/camera/status` - Device status updates
- `esp32/camera/command` - Commands to ESP32
- `esp32/camera/notification` - System notifications

## 🚀 Next Steps

1. Install and start Mosquitto
2. Restart backend server
3. Upload ESP32 MQTT code
4. Test image upload via MQTT

