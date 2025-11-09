require('dotenv').config();
const express = require('express');
const cors = require('cors');
const path = require('path');
const http = require('http');
const socketIO = require('socket.io');
const axios = require('axios');
const connectDB = require('./config/database');
const mqttService = require('./services/mqttService');

// Import routes
const authRoutes = require('./routes/auth');
const imageRoutes = require('./routes/images');

// Initialize Express app
const app = express();
const server = http.createServer(app);
const io = socketIO(server, {
  cors: {
    origin: '*',
    methods: ['GET', 'POST']
  }
});

// Connect to MongoDB
connectDB();

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Serve static files from uploads directory
app.use('/uploads', express.static(path.join(__dirname, 'uploads')));

// Request logging
app.use((req, res, next) => {
  console.log(`${new Date().toISOString()} - ${req.method} ${req.path}`);
  next();
});

// Routes
app.use('/api/auth', authRoutes);
app.use('/api', imageRoutes);

// Health check endpoint
app.get('/health', (req, res) => {
  res.status(200).json({
    success: true,
    message: 'Server is running',
    timestamp: new Date().toISOString()
  });
});

// Live stream proxy endpoint
app.get('/api/live', async (req, res) => {
  try {
    const esp32StreamUrl = process.env.ESP32_STREAM_URL;
    
    if (!esp32StreamUrl) {
      return res.status(503).json({
        success: false,
        message: 'ESP32 stream URL not configured'
      });
    }

    // Proxy MJPEG stream
    res.setHeader('Content-Type', 'multipart/x-mixed-replace; boundary=frame');
    
    const streamResponse = await axios({
      method: 'get',
      url: esp32StreamUrl,
      responseType: 'stream'
    });

    streamResponse.data.pipe(res);

    req.on('close', () => {
      streamResponse.data.destroy();
    });
  } catch (error) {
    console.error('Stream proxy error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error connecting to camera stream',
      error: error.message
    });
  }
});

// Socket.IO for real-time updates
io.on('connection', (socket) => {
  console.log('🔌 Client connected:', socket.id);

  // Send MQTT connection status
  socket.emit('mqtt-status', {
    connected: mqttService.isConnected()
  });

  socket.on('disconnect', () => {
    console.log('🔌 Client disconnected:', socket.id);
  });

  // Handle commands from frontend
  socket.on('send-command', (command) => {
    console.log('📤 Command from frontend:', command);
    mqttService.sendCommand(command);
  });
});

// Set Socket.IO for MQTT service
mqttService.setSocketIO(io);

// Make io accessible to routes
app.set('io', io);

// 404 handler
app.use((req, res) => {
  res.status(404).json({
    success: false,
    message: 'Route not found'
  });
});

// Global error handler
app.use((err, req, res, next) => {
  console.error('Error:', err.message);
  console.error(err.stack);

  // Multer file size error
  if (err.code === 'LIMIT_FILE_SIZE') {
    return res.status(400).json({
      success: false,
      message: 'File size is too large. Maximum size is 5MB'
    });
  }

  // Multer file type error
  if (err.message === 'Only JPEG images are allowed') {
    return res.status(400).json({
      success: false,
      message: err.message
    });
  }

  // JWT errors
  if (err.name === 'JsonWebTokenError') {
    return res.status(401).json({
      success: false,
      message: 'Invalid token'
    });
  }

  if (err.name === 'TokenExpiredError') {
    return res.status(401).json({
      success: false,
      message: 'Token expired'
    });
  }

  // MongoDB duplicate key error
  if (err.code === 11000) {
    const field = Object.keys(err.keyPattern)[0];
    return res.status(400).json({
      success: false,
      message: `${field} already exists`
    });
  }

  // Mongoose validation error
  if (err.name === 'ValidationError') {
    const errors = Object.values(err.errors).map(e => e.message);
    return res.status(400).json({
      success: false,
      message: 'Validation error',
      errors
    });
  }

  // Default error
  res.status(err.status || 500).json({
    success: false,
    message: err.message || 'Internal server error'
  });
});

// Start server
const PORT = process.env.PORT || 3000;

server.listen(PORT, async () => {
  console.log('='.repeat(50));
  console.log(`🚀 ESP32 Security Camera Backend Server`);
  console.log(`📡 Server running on port ${PORT}`);
  console.log(`🌍 Environment: ${process.env.NODE_ENV || 'development'}`);
  console.log(`🔗 Base URL: http://localhost:${PORT}`);
  console.log(`📸 Live Stream: http://localhost:${PORT}/api/live`);
  
  // Connect to MQTT broker
  await mqttService.connect();
  
  // Setup mDNS for ESP32 discovery (optional)
  try {
    const bonjour = require('bonjour')();
    bonjour.publish({
      name: 'esp32-server',
      type: 'http',
      port: PORT,
      txt: {
        path: '/api'
      }
    });
    console.log(`🔍 mDNS: Server discoverable at esp32-server.local:${PORT}`);
  } catch (error) {
    console.log(`ℹ️  mDNS not available (optional feature)`);
  }
  
  console.log('='.repeat(50));
});

// Handle unhandled promise rejections
process.on('unhandledRejection', (err) => {
  console.error('Unhandled Promise Rejection:', err.message);
  console.error(err.stack);
  // Close server & exit process
  server.close(() => process.exit(1));
});

// Handle SIGTERM
process.on('SIGTERM', () => {
  console.log('SIGTERM received. Shutting down gracefully...');
  server.close(() => {
    console.log('Process terminated');
  });
});
