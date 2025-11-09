# 🎥 ESP32 Security Camera System

Full-stack security camera system with ESP32-S3-EYE, featuring AI person detection, real-time streaming, and instant notifications.

## 📦 Project Structure

```
IOT/
├── CuoiKY/                      # Backend Server (Node.js + Express)
│   ├── app.js
│   ├── models/
│   ├── controllers/
│   ├── routes/
│   ├── uploads/
│   └── README.md
│
└── ESP32-Camera-Frontend/        # Frontend Web App (React + Vite)
    ├── src/
    │   ├── components/
    │   ├── pages/
    │   ├── services/
    │   └── context/
    ├── public/
    └── README.md
```

## 🚀 Quick Start

### 1. Backend Setup

```bash
cd CuoiKY

# Install dependencies
npm install

# Configure .env file
cp .env.example .env
# Edit .env with your MongoDB, Gmail, Telegram credentials

# Start server
npm start
# Server runs on http://localhost:3000
```

### 2. Frontend Setup

```bash
cd ESP32-Camera-Frontend

# Install dependencies
npm install
# or run install.bat

# Start development server
npm run dev
# or run start.bat
# Frontend runs on http://localhost:5173
```

### 3. Access Application

1. Open browser: `http://localhost:5173`
2. Register new account
3. Login and explore features

## ✨ Features

### Backend API
- ✅ RESTful API with Express.js
- ✅ MongoDB database with Mongoose
- ✅ JWT authentication & authorization
- ✅ Image upload & storage (Multer)
- ✅ AI person detection (OpenCV ready)
- ✅ Email alerts (Nodemailer + Gmail)
- ✅ Telegram notifications
- ✅ MJPEG stream proxy
- ✅ Socket.IO for real-time updates
- ✅ CORS enabled
- ✅ Docker support

### Frontend Web App
- ✅ Modern React 18 with Vite
- ✅ Beautiful UI with Tailwind CSS
- ✅ Dark/Light theme toggle
- ✅ Responsive design (mobile-first)
- ✅ Real-time live stream viewer
- ✅ Image gallery with search
- ✅ Dashboard with statistics
- ✅ Settings page for notifications
- ✅ JWT authentication flow
- ✅ Protected routes
- ✅ Error handling & loading states

## 🌐 System Architecture

```
┌─────────────┐      HTTP/WS       ┌──────────────┐       REST API      ┌─────────────┐
│   ESP32-    │ ──────────────────> │   Frontend   │ ─────────────────> │   Backend   │
│   S3-EYE    │   MJPEG Stream     │  React+Vite  │    JSON/FormData   │ Node+Express│
│   Camera    │                    │  Port: 5173  │                    │  Port: 3000 │
└─────────────┘                    └──────────────┘                    └─────────────┘
      │                                                                        │
      │                                                                        ├─> MongoDB
      │                                                                        ├─> Gmail
      └──────────────────────────────────────────────────────────────────────┴─> Telegram
                                  Upload Image (PIR Trigger)
```

## 🎯 Workflow

1. **PIR sensor detects motion** on ESP32
2. **ESP32 captures image** and sends to backend via HTTP POST
3. **Backend receives image**, saves to `/uploads`
4. **AI detection runs** (OpenCV - currently disabled)
5. **If person detected:**
   - Save metadata to MongoDB
   - Send email notification with image
   - Send Telegram message with image
6. **Frontend displays:**
   - Real-time dashboard updates
   - Image in gallery
   - Notifications

## 📸 ESP32 Integration

```cpp
// ESP32 sends image to backend
POST http://your-backend:3000/api/upload-image
Headers:
  Authorization: Bearer <JWT_TOKEN>
  Content-Type: multipart/form-data
Body:
  image: <JPEG_FILE>
```

See `examples/ESP32_EXAMPLE.cpp` for full implementation.

## 🔧 Configuration

### Backend (.env)
```bash
MONGO_URI=mongodb://localhost:27017/esp32_security
JWT_SECRET=your_secret_key
GMAIL_USER=your_email@gmail.com
GMAIL_PASS=your_app_password
TELEGRAM_TOKEN=your_bot_token
ESP32_STREAM_URL=http://192.168.1.100:81/stream
PORT=3000
```

### Frontend (.env)
```bash
VITE_API_URL=http://localhost:3000/api
```

## 📱 Screenshots

### Dashboard
- Real-time statistics
- Recent captures grid
- Quick navigation

### Live Stream
- MJPEG stream viewer
- Connection status indicator
- Reconnect functionality

### Gallery
- Paginated image grid
- Search functionality
- Image modal with details
- Delete images

### Settings
- Update email for alerts
- Configure Telegram Chat ID
- Theme preferences

## 🎨 Tech Stack

### Backend
- Node.js 18+
- Express.js
- MongoDB & Mongoose
- JWT (jsonwebtoken)
- Multer (file upload)
- Nodemailer (email)
- node-telegram-bot-api
- Socket.IO
- OpenCV (optional)

### Frontend
- React 18
- Vite
- React Router v6
- Tailwind CSS
- Axios
- Lucide Icons
- date-fns

## 🐳 Docker Deployment

```bash
# Backend
cd CuoiKY
docker-compose up -d

# Frontend (build for production)
cd ESP32-Camera-Frontend
npm run build
# Deploy dist/ folder to static hosting
```

## 📝 API Documentation

Full API documentation available in backend README:
- Authentication endpoints
- Image upload/retrieval
- User configuration
- Stream proxy

## 🔐 Security

- ✅ Passwords hashed with bcrypt
- ✅ JWT tokens with expiration
- ✅ Protected routes & middleware
- ✅ CORS configuration
- ✅ Input validation
- ✅ Error handling

## 🚀 Production Deployment

### Backend
1. Set `NODE_ENV=production`
2. Use MongoDB Atlas
3. Configure HTTPS/SSL
4. Use PM2 or Docker
5. Set up reverse proxy (nginx)

### Frontend
1. Build: `npm run build`
2. Deploy to:
   - Vercel
   - Netlify
   - GitHub Pages
   - AWS S3 + CloudFront

## 🐛 Common Issues

### Backend
- MongoDB connection: Check MONGO_URI
- Port conflict: Change PORT in .env
- Email not sending: Verify Gmail App Password
- Telegram not working: Check bot token & Chat ID

### Frontend
- API errors: Ensure backend is running
- CORS issues: Check backend CORS config
- Build errors: Clear node_modules, reinstall
- Stream not loading: Verify ESP32_STREAM_URL

## 📚 Documentation

- [Backend README](./CuoiKY/README.md)
- [Frontend README](./ESP32-Camera-Frontend/README.md)
- [ESP32 Example](./CuoiKY/examples/ESP32_EXAMPLE.cpp)
- [Quick Start Guide](./CuoiKY/QUICKSTART.md)

## 🤝 Contributing

1. Fork the repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Open Pull Request

## 📄 License

ISC License

## 👨‍💻 Authors

ESP32 Security Camera System - Full Stack Project

---

**Happy Coding! 🎉**
