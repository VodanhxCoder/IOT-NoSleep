@echo off
echo ========================================
echo  Starting ESP32 Camera System with MQTT
echo ========================================

REM Check if Docker is running
docker info >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Docker Desktop is not running!
    echo Please start Docker Desktop first.
    echo.
    echo Alternative: Run without Docker
    echo   1. Install Node.js
    echo   2. Run: npm install
    echo   3. Run: npm start
    pause
    exit /b 1
)

REM Check for .env file
if not exist ".env" (
    echo [WARNING] .env file not found!
    echo Creating .env from example...
    copy .env.example .env
    echo Please edit .env file with your configuration.
    pause
)

REM Create necessary directories
if not exist "mosquitto\data" mkdir mosquitto\data
if not exist "mosquitto\log" mkdir mosquitto\log
if not exist "uploads" mkdir uploads
if not exist "logs" mkdir logs

echo.
echo Starting services...
docker-compose -f docker-compose.mqtt.yml up -d

if errorlevel 0 (
    echo.
    echo ========================================
    echo  Services Started Successfully!
    echo ========================================
    echo.
    echo  MQTT Broker: mqtt://localhost:1883
    echo  Backend API: http://localhost:3000
    echo  MQTT WebSocket: ws://localhost:9001
    echo.
    echo To view logs:
    echo   docker-compose -f docker-compose.mqtt.yml logs -f
    echo.
    echo To stop:
    echo   docker-compose -f docker-compose.mqtt.yml down
    echo.
    pause
) else (
    echo.
    echo [ERROR] Failed to start services!
    echo Check Docker Desktop and try again.
    pause
    exit /b 1
)
