# ESP32 Security Camera - Frontend

Modern React frontend application for ESP32-S3-EYE security camera system with real-time streaming, image gallery, and AI detection alerts.

## 🚀 Features

- **Real-time Live Stream** - View MJPEG stream from ESP32 camera
- **Image Gallery** - Browse all captured images with search and pagination
- **Dashboard** - Quick overview with statistics and recent captures
- **Dark Mode** - Beautiful light/dark theme with smooth transitions
- **Responsive Design** - Works on desktop, tablet, and mobile
- **Authentication** - Secure JWT-based login/register
- **Notifications** - Configure email and Telegram alerts
- **Modern UI** - Clean interface built with Tailwind CSS and Lucide icons

## 📋 Prerequisites

- Node.js >= 18.x
- npm or yarn
- Backend server running on `http://localhost:3000`

## 🛠️ Installation

1. **Install dependencies:**
   ```bash
   npm install
   ```

2. **Configure environment (optional):**
   
   Create `.env` file if you need to customize:
   ```bash
   VITE_API_URL=http://localhost:3000/api
   ```

3. **Start development server:**
   ```bash
   npm run dev
   ```

4. **Build for production:**
   ```bash
   npm run build
   ```

5. **Preview production build:**
   ```bash
   npm run preview
   ```

## 🌐 Usage

1. Open browser to `http://localhost:5173`
2. Register a new account or login
3. **Dashboard** - View statistics and recent captures
4. **Live Stream** - Watch real-time camera feed
5. **Gallery** - Browse, search, and delete images
6. **Settings** - Configure email/Telegram notifications

## 🎨 Tech Stack

- **React 18** - UI framework
- **Vite** - Build tool and dev server
- **React Router** - Client-side routing
- **Tailwind CSS** - Utility-first styling
- **Lucide React** - Beautiful icon set
- **Axios** - HTTP client
- **date-fns** - Date formatting

## 📁 Project Structure

```
src/
├── components/          # Reusable components
│   └── Navbar.jsx      # Navigation bar with theme toggle
├── context/            # React contexts
│   ├── ThemeContext.jsx   # Dark/light mode management
│   └── AuthContext.jsx    # Authentication state
├── pages/              # Route pages
│   ├── Login.jsx      # Login page
│   ├── Register.jsx   # Registration page
│   ├── Dashboard.jsx  # Main dashboard
│   ├── LiveStream.jsx # Live camera feed
│   ├── Gallery.jsx    # Image gallery
│   └── Settings.jsx   # User settings
├── services/           # API services
│   ├── api.js         # Axios instance with interceptors
│   ├── authService.js # Authentication API calls
│   └── imageService.js # Image API calls
├── App.jsx            # Main app component
├── main.jsx           # Entry point
└── index.css          # Global styles

```

## 🎯 API Integration

Frontend connects to backend via axios with:
- **Base URL**: `http://localhost:3000/api`
- **Authentication**: JWT token in Authorization header
- **Auto-retry**: Failed requests handled gracefully
- **Error handling**: User-friendly error messages

## 🌙 Dark Mode

Toggle between light and dark themes:
- Persists preference in localStorage
- Smooth transitions between themes
- Applies to all components

## 🔐 Authentication Flow

1. User registers/logs in
2. JWT token stored in localStorage
3. Token automatically added to API requests
4. Protected routes redirect to login if unauthorized

## 📱 Responsive Design

- **Mobile**: Hamburger menu, stacked layout
- **Tablet**: Grid layout for images
- **Desktop**: Full navigation, multi-column grids

## 🚀 Deployment

1. **Build production bundle:**
   ```bash
   npm run build
   ```

2. **Deploy `dist/` folder** to:
   - Netlify
   - Vercel
   - GitHub Pages
   - Any static hosting

3. **Update API URL** in production:
   ```bash
   VITE_API_URL=https://your-backend-api.com/api
   ```

## 🔧 Troubleshooting

**CORS errors:**
- Ensure backend has CORS enabled for frontend origin

**Live stream not working:**
- Check ESP32_STREAM_URL in backend .env
- Verify ESP32 camera is online and streaming

**Images not loading:**
- Confirm backend uploads/ directory is accessible
- Check image paths in API responses

## 📄 License

ISC

## 👨‍💻 Author

ESP32 Security Camera Project

---

**Note**: Make sure backend server is running before starting frontend!
