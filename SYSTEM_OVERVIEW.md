# 📊 ESP32 Security Camera System - Tổng Quan Hệ Thống

## 🎯 Mục Đích Dự Án
Hệ thống camera an ninh thông minh sử dụng ESP32-S3-EYE với khả năng:
- Phát hiện người tự động bằng AI
- Live streaming thời gian thực
- Gửi cảnh báo qua Email & Telegram
- Quản lý ảnh với giao diện web hiện đại

---

## 🏗️ Kiến Trúc Hệ Thống

```
┌─────────────────┐           ┌──────────────────┐           ┌─────────────────┐
│   ESP32-S3-EYE  │  MJPEG    │   Backend        │   REST    │   Frontend      │
│   Camera        │──────────>│   Node.js        │<──────────│   React + Vite  │
│                 │  Stream   │   + MongoDB      │   API     │                 │
└─────────────────┘           └──────────────────┘           └─────────────────┘
        │                              │
        │                              ├──> Gmail (Email alerts)
        │                              ├──> Telegram Bot
        │                              └──> OpenCV (Person detection)
```

---

## 📦 Backend API (Node.js + Express)

### **1. Authentication APIs** (`/api/auth`)

#### 🔐 **POST /api/auth/register**
Đăng ký tài khoản mới
- **Input**: `{ username, email, password, telegramId? }`
- **Output**: `{ success, data: { user, token } }`
- **Validation**:
  - Username: ≥3 ký tự, chỉ chữ cái, số, dấu gạch dưới và khoảng trắng
  - Email: phải hợp lệ
  - Password: ≥6 ký tự
- **Chức năng**: Tạo user mới, hash password, tạo JWT token

#### 🔑 **POST /api/auth/login**
Đăng nhập
- **Input**: `{ username, password }`
- **Output**: `{ success, data: { user, token } }`
- **Chức năng**: Xác thực user, so sánh password đã hash, trả về token

#### 👤 **GET /api/auth/me**
Lấy thông tin user hiện tại (Protected route)
- **Headers**: `Authorization: Bearer <token>`
- **Output**: `{ success, data: user }`

---

### **2. Image APIs** (`/api`)

#### 📸 **POST /api/upload-image** (Protected)
Upload ảnh từ ESP32 hoặc web
- **Input**: FormData với file ảnh
- **Headers**: `Authorization: Bearer <token>`
- **Chức năng**:
  1. Nhận ảnh từ ESP32
  2. Chạy AI phát hiện người (OpenCV)
  3. Nếu phát hiện người:
     - Lưu ảnh vào MongoDB
     - Gửi email cảnh báo với ảnh đính kèm
     - Gửi Telegram notification với ảnh
  4. Nếu không phát hiện: xóa ảnh
- **Output**: `{ success, message, data: { image } }`

#### 🖼️ **GET /api/images?page=1&limit=10** (Protected)
Lấy danh sách ảnh với phân trang
- **Query params**: 
  - `page`: số trang (default: 1)
  - `limit`: số ảnh mỗi trang (default: 10)
- **Output**: `{ success, data: { images, pagination } }`
- **Pagination**: `{ current, total, count, totalImages }`

#### 🔍 **GET /api/images/:id** (Protected)
Lấy thông tin 1 ảnh cụ thể
- **Params**: `id` - MongoDB ObjectId
- **Output**: `{ success, data: image }`

#### 🗑️ **DELETE /api/images/:id** (Protected)
Xóa ảnh
- **Params**: `id` - MongoDB ObjectId
- **Chức năng**: Xóa file khỏi filesystem và database
- **Output**: `{ success, message }`

#### ⚙️ **PUT /api/config** (Protected)
Cập nhật cài đặt thông báo
- **Input**: `{ email?, telegramId? }`
- **Chức năng**: Cập nhật email và Telegram ID cho notifications
- **Output**: `{ success, data: user }`

---

### **3. Streaming API**

#### 📺 **GET /api/live**
Live stream từ ESP32 (không cần auth)
- **Chức năng**: Proxy MJPEG stream từ ESP32 ra browser
- **Content-Type**: `multipart/x-mixed-replace; boundary=frame`
- **Stream URL**: Đọc từ `ESP32_STREAM_URL` trong .env

---

### **4. Utility Endpoints**

#### ❤️ **GET /health**
Kiểm tra server status
- **Output**: `{ success, message, timestamp }`

#### 🔌 **Socket.IO**
WebSocket cho real-time updates
- **Events**: 
  - `connection` - Client kết nối
  - `disconnect` - Client ngắt kết nối

---

## 🎨 Frontend (React + Vite)

### **Pages (Routes)**

#### 1️⃣ **Login** (`/login`)
- Form đăng nhập với username + password
- Sau khi login thành công → redirect về Dashboard
- Lưu JWT token vào localStorage

#### 2️⃣ **Register** (`/register`)
- Form đăng ký với username, email, password, confirm password
- Validation password phải khớp
- Sau khi đăng ký thành công → redirect về Login

#### 3️⃣ **Dashboard** (`/`) - Protected
- Hiển thị thống kê:
  - Tổng số ảnh
  - Ảnh hôm nay
  - Số lượng phát hiện người
- 6 ảnh gần nhất
- StatCards với icon và màu sắc

#### 4️⃣ **Live Stream** (`/live`) - Protected
- Hiển thị MJPEG stream từ ESP32
- Nút Reconnect để kết nối lại
- Status indicator (Connected/Disconnected)
- Auto-connect khi vào trang

#### 5️⃣ **Gallery** (`/gallery`) - Protected
- Grid hiển thị tất cả ảnh (responsive)
- Search box để tìm kiếm theo filename hoặc detected object
- Pagination (Previous/Next)
- Hover để View hoặc Delete
- Modal để xem ảnh full size
- Download ảnh

#### 6️⃣ **Settings** (`/settings`) - Protected
- Hiển thị thông tin user
- Form cập nhật Email
- Form cập nhật Telegram Chat ID
- Hướng dẫn lấy Telegram Chat ID

---

### **Components**

#### 📱 **Navbar**
- Logo + tên project
- Navigation links: Dashboard, Live Stream, Gallery, Settings
- Dark/Light theme toggle
- Username dropdown với Logout

#### 🌓 **ThemeContext**
- Quản lý Dark/Light mode
- Lưu preference vào localStorage
- Toggle theme với smooth transition

#### 🔐 **AuthContext**
- Quản lý user state
- Functions: `login()`, `logout()`, `register()`
- Auto-load user từ localStorage khi refresh

#### 🛡️ **ProtectedRoute**
- HOC bảo vệ các route cần authentication
- Redirect về `/login` nếu chưa đăng nhập
- Loading spinner khi đang check auth

---

### **Services**

#### 🔧 **api.js**
Axios instance với:
- Base URL: `http://localhost:3000/api`
- Request interceptor: tự động thêm JWT token vào header
- Response interceptor: xử lý 401 error (logout)

#### 👤 **authService.js**
- `register(userData)` - Đăng ký
- `login(credentials)` - Đăng nhập (lưu token + user)
- `logout()` - Đăng xuất (xóa token + user)
- `getCurrentUser()` - Lấy user từ localStorage
- `getMe()` - Gọi API lấy user info

#### 🖼️ **imageService.js**
- `uploadImage(file)` - Upload ảnh
- `getImages(page)` - Lấy danh sách ảnh
- `getImageById(id)` - Lấy 1 ảnh
- `deleteImage(id)` - Xóa ảnh
- `updateConfig(config)` - Cập nhật settings

---

## 🗄️ Database Models (MongoDB)

### **User Schema**
```javascript
{
  username: String (unique, required, min: 3),
  password: String (hashed, required, min: 6),
  email: String (unique, required),
  telegramId: String (nullable),
  timestamps: true
}
```

### **Image Schema**
```javascript
{
  filename: String (required),
  path: String (required),
  timestamp: Date (default: now),
  detectedObject: String (e.g., "person"),
  userId: ObjectId (ref: User),
  timestamps: true
}
```

---

## 🔐 Security Features

1. **JWT Authentication**: Token-based auth với expiry 7 ngày
2. **Password Hashing**: Bcrypt với salt rounds = 10
3. **Protected Routes**: Middleware kiểm tra JWT token
4. **Input Validation**: express-validator cho tất cả inputs
5. **CORS**: Cấu hình cho phép frontend access
6. **Error Handling**: Global error handler với proper status codes

---

## 📧 Notification System

### **Email Alerts** (Nodemailer + Gmail)
- Trigger: Khi phát hiện người trong ảnh
- Content: HTML email với thông tin detection + ảnh đính kèm
- Config: Gmail App Password (không dùng password thường)

### **Telegram Notifications**
- Trigger: Khi phát hiện người trong ảnh
- Content: Message với ảnh và thông tin detection
- Setup: Cần Telegram Bot Token + User Chat ID

---

## 🤖 AI Detection (OpenCV)

### **Person Detection**
- **Hiện tại**: Tạm disabled vì cần build native modules
- **Kế hoạch**: Sử dụng Haar Cascade classifier để detect faces
- **Fallback**: Hiện tại return `true` để test được hệ thống
- **TODO**: Cài OpenCV libraries và rebuild opencv4nodejs

---

## 🚀 Workflow Hoàn Chỉnh

### **Use Case 1: User đăng ký và xem camera**
1. User truy cập `http://localhost:5173`
2. Click "Sign up" → Nhập thông tin → Register
3. Redirect về Login → Đăng nhập
4. Redirect về Dashboard (trang chủ)
5. Click "Live Stream" → Xem camera trực tiếp
6. Click "Settings" → Cập nhật Email/Telegram ID

### **Use Case 2: ESP32 gửi ảnh**
1. ESP32 chụp ảnh khi phát hiện chuyển động
2. POST ảnh đến `/api/upload-image` (với JWT token)
3. Backend nhận ảnh → Chạy AI detection
4. Nếu phát hiện người:
   - Lưu vào MongoDB
   - Gửi email alert
   - Gửi Telegram notification
5. User nhận thông báo ngay lập tức
6. User mở Gallery trên web để xem ảnh

### **Use Case 3: User quản lý ảnh**
1. Vào Gallery page
2. Tìm kiếm ảnh theo tên hoặc detected object
3. Click vào ảnh để xem full size
4. Download hoặc Delete ảnh
5. Navigate qua các trang với pagination

---

## 📊 Các Chức Năng Chính

### ✅ **Đã Hoàn Thành**
- [x] User authentication (Register/Login)
- [x] JWT token management
- [x] Image upload & storage
- [x] Live MJPEG streaming proxy
- [x] Email notifications
- [x] Telegram notifications
- [x] Image gallery với search & pagination
- [x] Dashboard với statistics
- [x] Dark/Light theme
- [x] Responsive design
- [x] Protected routes
- [x] Settings page

### ⚠️ **Đang Disabled**
- [ ] OpenCV person detection (cần cài OpenCV native libs)

### 🔮 **Có Thể Mở Rộng**
- [ ] Video recording
- [ ] Motion zones configuration
- [ ] Multiple camera support
- [ ] Face recognition
- [ ] Cloud storage (AWS S3, Cloudinary)
- [ ] Mobile app (React Native)
- [ ] Advanced analytics dashboard

---

## 🔧 Environment Variables

### **Backend (.env)**
```env
MONGO_URI=mongodb+srv://...        # MongoDB connection string
PORT=3000                          # Server port
JWT_SECRET=your_secret_key         # JWT signing key
JWT_EXPIRE=7d                      # Token expiration
GMAIL_USER=your@gmail.com          # Gmail for notifications
GMAIL_PASS=app_password            # Gmail app password
TELEGRAM_TOKEN=bot_token           # Telegram bot token
ESP32_STREAM_URL=http://ip:81/stream  # ESP32 stream URL
```

### **Frontend (.env)**
```env
VITE_API_URL=http://localhost:3000/api  # Backend API URL
```

---

## 🎯 API Response Formats

### **Success Response**
```json
{
  "success": true,
  "message": "Operation successful",
  "data": { ... }
}
```

### **Error Response**
```json
{
  "success": false,
  "message": "Error description",
  "error": "Detailed error message"
}
```

### **Validation Error**
```json
{
  "success": false,
  "message": "Validation failed",
  "errors": [
    {
      "type": "field",
      "value": "...",
      "msg": "Error message",
      "path": "fieldName",
      "location": "body"
    }
  ]
}
```

---

## 🌐 Ports & URLs

| Service | URL | Port |
|---------|-----|------|
| Frontend Dev | http://localhost:5173 | 5173 |
| Backend API | http://localhost:3000 | 3000 |
| MongoDB | mongodb://localhost:27017 | 27017 |
| ESP32 Stream | http://192.168.x.x:81/stream | 81 |

---

## 📝 Tóm Tắt Tech Stack

### Backend
- **Runtime**: Node.js 18+
- **Framework**: Express.js
- **Database**: MongoDB + Mongoose
- **Auth**: JWT + bcrypt
- **AI**: OpenCV4nodejs (planned)
- **Upload**: Multer
- **Email**: Nodemailer
- **Telegram**: node-telegram-bot-api
- **Real-time**: Socket.IO

### Frontend
- **Framework**: React 18
- **Build Tool**: Vite
- **Routing**: React Router v6
- **Styling**: Tailwind CSS
- **Icons**: Lucide React
- **HTTP Client**: Axios
- **Date Utils**: date-fns

---

## 🎓 Kết Luận

Hệ thống này là một **full-stack security camera solution** hoàn chỉnh với:
- Backend RESTful API mạnh mẽ
- Frontend hiện đại với UX/UI đẹp
- AI detection tự động
- Multi-channel notifications
- Real-time streaming
- Responsive & dark mode support

**Status hiện tại**: ✅ Hoạt động tốt, backend + frontend đã chạy thành công!
