@echo off
title ESP32 Camera Backend with Local MQTT

echo ========================================
echo  ESP32 Camera Backend + Local MQTT
echo ========================================
echo.

REM Check Node.js
node --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Node.js not found!
    echo Please install Node.js from https://nodejs.org
    pause
    exit /b 1
)

REM Check MQTT broker on port 1883
netstat -ano | findstr :1883 >nul
if errorlevel 1 (
    echo [WARNING] No MQTT broker detected on port 1883
    echo.
    echo Options:
    echo   1. Start Docker MQTT: run start-mqtt.bat
    echo   2. Use public broker (edit .env: MQTT_BROKER=mqtt://broker.hivemq.com:1883)
    echo   3. Install Mosquitto manually
    echo.
    echo Continuing anyway...
    timeout /t 3
)

REM Set environment for local MQTT
set MQTT_BROKER=mqtt://localhost:1883
set NODE_ENV=development

echo.
echo Starting backend server...
echo MQTT Broker: %MQTT_BROKER%
echo.

node app.js

pause
