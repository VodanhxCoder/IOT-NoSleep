# ESP32 Security Camera Backend

Backend server cho hệ thống camera giám sát ESP32-CAM với tính năng livestream, lưu trữ hình ảnh và điều khiển từ xa qua MQTT.

## Yêu cầu hệ thống

- Node.js (v14 trở lên)
- MongoDB (v4.4 trở lên)
- MQTT Broker (Mosquitto hoặc tương tự)

## Cài đặt

1. Clone repository:
```bash
git clone <repository-url>
cd ESP32-Camera-Backend
```

2. Cài đặt dependencies:
```bash
npm install
```

3. Tạo file `.env` từ file mẫu:
```bash
cp .env.example .env
```

4. Chỉnh sửa file `.env` với thông tin cấu hình của bạn

5. Tạo thư mục uploads:
```bash
mkdir uploads
```

## Chạy ứng dụng

### Development mode:
```bash
npm run dev
```

### Production mode:
```bash
npm start
```

## API Endpoints

### Authentication
- `POST /api/auth/register` - Đăng ký tài khoản
- `POST /api/auth/login` - Đăng nhập

### Images
- `GET /api/images` - Lấy danh sách ảnh
- `POST /api/images/upload` - Upload ảnh
- `DELETE /api/images/:id` - Xóa ảnh

### Live Stream
- `GET /api/live` - Xem livestream từ ESP32-CAM

### Health Check
- `GET /health` - Kiểm tra trạng thái server

## Cấu trúc thư mục
```
ESP32-Camera-Backend/
├── app.js                      # Main application entry point
├── package.json                # Dependencies and scripts
├── .env.example                # Environment variables template
├── .gitignore                  # Git ignore rules
├── Dockerfile                  # Docker image configuration
├── docker-compose.yml          # Multi-container setup
├── config/
│   └── database.js            # MongoDB connection
├── models/
│   ├── User.js                # User schema (username, email, password, telegramId)
│   └── Image.js               # Image schema (filename, path, timestamp, detectedObject)
├── controllers/
│   ├── authController.js      # Authentication logic (register, login)
│   └── imageController.js     # Image processing and notifications
├── routes/
│   ├── auth.js                # Auth routes (/register, /login)
│   └── images.js              # Image routes (/upload-image, /images, /config)
├── middlewares/
│   └── auth.js                # JWT verification middleware
└── uploads/                    # Uploaded images directory
```

## ESP32 Integration

### Example ESP32 Code to Upload Image

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include "esp_camera.h"

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* serverUrl = "http://YOUR_SERVER_IP:3000/api/upload-image";
const char* jwtToken = "YOUR_JWT_TOKEN";

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  
  // Initialize camera
  camera_config_t config;
  // ... configure camera pins for ESP32-S3-EYE
  esp_camera_init(&config);
}

void uploadImage() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Authorization", String("Bearer ") + jwtToken);
  
  int httpResponseCode = http.POST(fb->buf, fb->len);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);
  }
  
  esp_camera_fb_return(fb);
  http.end();
}

void loop() {
  // Trigger on PIR sensor or motion detection
  if (digitalRead(PIR_PIN) == HIGH) {
    uploadImage();
    delay(5000); // Cooldown
  }
}
```

## Troubleshooting

### OpenCV Installation Issues

If `opencv4nodejs` fails to install:

**Windows**:
```bash
npm install --global windows-build-tools
npm install opencv4nodejs
```

**Linux**:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake
npm install opencv4nodejs
```

**Alternative**: Use Docker (OpenCV dependencies included in Dockerfile)

### MongoDB Connection Issues

- **Local**: Ensure MongoDB service is running
- **Atlas**: Check IP whitelist and connection string
- **Docker**: Use `mongodb://mongo:27017/esp32_security`

### Port Already in Use

Change port in `.env`:
```env
PORT=3001
```

## Security Notes

- Never commit `.env` file (already in `.gitignore`)
- Change `JWT_SECRET` to a strong random string
- Use Gmail App Passwords, not account password
- Implement rate limiting for production
- Use HTTPS in production (reverse proxy with nginx/Caddy)

## Testing

### Test with cURL

**Register**:
```bash
curl -X POST http://localhost:3000/api/auth/register \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123","email":"admin@example.com"}'
```

**Login**:
```bash
curl -X POST http://localhost:3000/api/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username":"admin","password":"admin123"}'
```

**Upload Image**:
```bash
curl -X POST http://localhost:3000/api/upload-image \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -F "image=@test.jpg"
```

## Docker Commands

```bash
# Build and start
docker-compose up -d

# View logs
docker-compose logs -f

# Restart services
docker-compose restart

# Stop and remove
docker-compose down

# Remove volumes (delete data)
docker-compose down -v

# Rebuild images
docker-compose build --no-cache
```

## Production Deployment

1. Set `NODE_ENV=production` in `.env`
2. Use strong `JWT_SECRET`
3. Configure firewall rules
4. Use reverse proxy (nginx) with SSL
5. Enable MongoDB authentication
6. Implement rate limiting
7. Set up monitoring (PM2, Docker health checks)
8. Regular backups of MongoDB

## License

MIT

## Support

For issues or questions, please open an issue on the repository.

---

**Built with ❤️ for ESP32-S3-EYE Security Camera System**
