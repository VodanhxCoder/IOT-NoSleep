# 🚀 ESP32 Security Camera Backend - Hướng dẫn sử dụng

## ✅ Dự án đã được thiết lập thành công!

### 📊 Trạng thái hiện tại:
- ✅ Server: Đang chạy trên `http://localhost:3000`
- ✅ MongoDB: Kết nối thành công với MongoDB Atlas
- ✅ Database: `esp32_security`
- ✅ Dependencies: Đã cài đặt (327 packages)

---

# Quick Start Guide - ESP32 Security Camera Backend

## Setup Instructions

### 1. Install Dependencies

```bash
npm install
```

### 2. Configure Environment

Copy the example environment file:
```bash
copy .env.example .env
```

Edit `.env` with your settings:
- Set `MONGO_URI` to your MongoDB connection
- Generate strong `JWT_SECRET`
- Add Gmail credentials (enable App Password in Gmail)
- Add Telegram bot token (optional, from @BotFather)

### 3. Start MongoDB

#### Option A: Local MongoDB
```bash
# Windows
net start MongoDB

# Or if installed manually
mongod --dbpath C:\data\db
```

#### Option B: Docker
```bash
docker run -d -p 27017:27017 --name mongo mongo:latest
```

#### Option C: MongoDB Atlas
Use cloud MongoDB and update MONGO_URI in .env

### 4. Run the Application

```bash
# Development mode (with auto-reload)
npm run dev

# Production mode
npm start
```

Server will start at `http://localhost:3000`

### 5. Test the API

#### Register a user:
```bash
curl -X POST http://localhost:3000/api/auth/register ^
  -H "Content-Type: application/json" ^
  -d "{\"username\":\"admin\",\"password\":\"admin123\",\"email\":\"your@email.com\"}"
```

#### Login:
```bash
curl -X POST http://localhost:3000/api/auth/login ^
  -H "Content-Type: application/json" ^
  -d "{\"username\":\"admin\",\"password\":\"admin123\"}"
```

Save the token from the response!

#### Upload an image:
```bash
curl -X POST http://localhost:3000/api/upload-image ^
  -H "Authorization: Bearer YOUR_TOKEN_HERE" ^
  -F "image=@test.jpg"
```

## Docker Deployment

### Development:
```bash
# Copy environment file
copy .env.example .env

# Start services
docker-compose -f docker-compose.dev.yml up -d

# View logs
docker-compose -f docker-compose.dev.yml logs -f
```

### Production:
```bash
# Copy environment file
copy .env.example .env

# Start services
docker-compose up -d

# View logs
docker-compose logs -f backend
```

## Troubleshooting

### Port 3000 already in use
Change PORT in .env to another value (e.g., 3001)

### MongoDB connection failed
- Check if MongoDB is running
- Verify MONGO_URI in .env
- For Docker: ensure mongo service is healthy

### OpenCV installation issues
Use Docker deployment which includes all dependencies

### Email not sending
- Enable 2FA in Gmail
- Generate App Password (not your regular password)
- Check GMAIL_USER and GMAIL_PASS in .env

### Telegram notifications not working
- Get bot token from @BotFather
- Get your Telegram ID from @userinfobot
- Set TELEGRAM_TOKEN in .env
- Add telegramId when registering or via /api/config endpoint

## Next Steps

1. Configure ESP32 to send images (see ESP32_EXAMPLE.cpp)
2. Test with Postman or the included test-api.js script
3. Set up frontend application
4. Deploy to production server
5. Configure SSL with reverse proxy (nginx/Caddy)

## Support

Check README.md for full documentation and troubleshooting guide.
