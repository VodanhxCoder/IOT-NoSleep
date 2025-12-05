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
const configRoutes = require('./routes/config');
const userRoutes = require('./routes/users');

// Initialize Express app
const app = express();
const server = http.createServer(app);
const io = socketIO(server, {
  cors: {
    origin: '*',
    methods: ['GET', 'POST']
  }
});

// Make io accessible to our router
app.set('io', io);

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
app.use('/api/users', userRoutes);
app.use('/api/config', configRoutes);
app.use('/api', imageRoutes);

// Health check endpoint
app.get('/health', (req, res) => {
  res.status(200).json({
    success: true,
    message: 'Server is running',
    timestamp: new Date().toISOString()
  });
});

// Stream Broadcaster State
let streamClients = [];
let wsStreamClients = new Set(); // WebSocket Clients
let activeStreamRequest = null;
let activeStreamResponse = null;

// Live stream proxy endpoint (Broadcaster Mode)
app.get('/api/live', (req, res) => {
  const esp32StreamUrl = process.env.ESP32_STREAM_URL;
  
  if (!esp32StreamUrl) {
    return res.status(503).json({
      success: false,
      message: 'ESP32 stream URL not configured. Waiting for ESP32 to connect via MQTT.'
    });
  }

  // 1. Setup Client Response
  res.setHeader('Content-Type', 'multipart/x-mixed-replace; boundary=123456789000000000000987654321');
  res.setHeader('Connection', 'close');
  res.setHeader('Cache-Control', 'no-cache, no-store, must-revalidate');
  res.setHeader('Pragma', 'no-cache');
  res.setHeader('Expires', '0');

  // Add client to list
  const clientId = Math.random().toString(36).substring(7);
  console.log(`Stream Client connected: ${clientId}`);
  streamClients.push({ id: clientId, res });

  // 2. Ensure Connection to ESP32
  startStreamIfNeeded();

  // 3. Handle Client Disconnect
  req.on('close', () => {
    console.log(`Stream Client disconnected: ${clientId}`);
    streamClients = streamClients.filter(c => c.id !== clientId);
    checkAndCloseStream();
  });
});

async function startStreamIfNeeded() {
  if (activeStreamRequest) return;

  const esp32StreamUrl = process.env.ESP32_STREAM_URL;
  if (!esp32StreamUrl) return;

  console.log(`Initiating Master Connection to ESP32: ${esp32StreamUrl}`);
    
  try {
    const controller = new AbortController();
    activeStreamRequest = controller;

    const response = await axios({
      method: 'get',
      url: esp32StreamUrl,
      responseType: 'stream',
      timeout: 5000,
      signal: controller.signal
    });

    activeStreamResponse = response.data;
    console.log('Master Connection Established');

    let frameBuffer = Buffer.alloc(0);
    const SOI = Buffer.from([0xff, 0xd8]); // Start of Image
    const EOI = Buffer.from([0xff, 0xd9]); // End of Image

    // Broadcast data to all clients
    response.data.on('data', (chunk) => {
      // 1. Send to HTTP Stream Clients (MJPEG) - DEPRECATED but kept for compatibility if needed
      if (streamClients.length > 0) {
          streamClients.forEach(client => {
            try {
              client.res.write(chunk);
            } catch (e) {
              // console.error(`❌ Error writing to client ${client.id}:`, e.message);
            }
          });
      }

      // 2. Send to WebSocket Clients (Binary Frames)
      if (wsStreamClients.size > 0) {
          frameBuffer = Buffer.concat([frameBuffer, chunk]);

          let soiIndex = frameBuffer.indexOf(SOI);
          let eoiIndex = frameBuffer.indexOf(EOI);

          while (soiIndex !== -1 && eoiIndex !== -1) {
              if (eoiIndex < soiIndex) {
                  frameBuffer = frameBuffer.slice(soiIndex);
                  soiIndex = 0;
                  eoiIndex = frameBuffer.indexOf(EOI);
                  continue;
              }

              const frame = frameBuffer.slice(soiIndex, eoiIndex + 2);
              io.emit('stream-frame', frame);

              frameBuffer = frameBuffer.slice(eoiIndex + 2);
              soiIndex = frameBuffer.indexOf(SOI);
              eoiIndex = frameBuffer.indexOf(EOI);
          }
      }
    });

    // Handle ESP32 Disconnect
    response.data.on('end', () => {
      console.log('ESP32 Stream Ended');
      cleanupMasterConnection();
    });

    response.data.on('error', (err) => {
      console.error('ESP32 Stream Error:', err.message);
      cleanupMasterConnection();
    });

  } catch (error) {
    console.error('Failed to connect to ESP32:', error.message);
    cleanupMasterConnection();
    
    // Notify all waiting clients
    streamClients.forEach(client => {
        try { client.res.status(502).end(); } catch(e) {}
    });
    streamClients = [];
  }
}

function cleanupMasterConnection() {
  if (activeStreamResponse) {
    activeStreamResponse.destroy();
    activeStreamResponse = null;
  }
  if (activeStreamRequest) {
    activeStreamRequest.abort();
    activeStreamRequest = null;
  }
}

// Socket.IO for real-time updates
io.on('connection', (socket) => {
  console.log('Client connected:', socket.id);

  // Send MQTT connection status
  socket.emit('mqtt-status', {
    connected: mqttService.isConnected()
  });

  // Send last known ESP32 status immediately
  const lastStatus = mqttService.getLastStatus();
  if (lastStatus && lastStatus.status !== 'unknown') {
      socket.emit('esp32-status', lastStatus);
  }

  socket.on('join-stream', () => {
    console.log(`WS Client joined stream: ${socket.id}`);
    wsStreamClients.add(socket.id);
    startStreamIfNeeded();
  });

  socket.on('leave-stream', () => {
    console.log(`WS Client left stream: ${socket.id}`);
    wsStreamClients.delete(socket.id);
    checkAndCloseStream();
  });

  socket.on('disconnect', () => {
    console.log('Client disconnected:', socket.id);
    if (wsStreamClients.has(socket.id)) {
        wsStreamClients.delete(socket.id);
        checkAndCloseStream();
    }
  });

  // Handle commands from frontend
  socket.on('send-command', (command) => {
    console.log('Command from frontend:', command);
    mqttService.sendCommand(command);
  });
});

function checkAndCloseStream() {
    if (streamClients.length === 0 && wsStreamClients.size === 0) {
      console.log('No clients left (HTTP or WS). Waiting 2s before closing Master Connection...');
      setTimeout(() => {
        if (streamClients.length === 0 && wsStreamClients.size === 0 && activeStreamRequest) {
          console.log('Closing Master Connection (Idle)');
          cleanupMasterConnection();
        }
      }, 2000);
    }
}

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
  console.log(`ESP32 Security Camera Backend Server`);
  console.log(`Server running on port ${PORT}`);
  console.log(`Environment: ${process.env.NODE_ENV || 'development'}`);
  console.log(`Base URL: http://localhost:${PORT}`);
  console.log(`Live Stream: http://localhost:${PORT}/api/live`);
  
  // Connect to MQTT broker
  await mqttService.connect();
  
  // Setup mDNS for ESP32 discovery (optional)
  try {
    // Helper to find the correct local IP (WiFi) and avoid WSL/Docker/Hotspot IPs
    const getLocalIP = () => {
      const os = require('os');
      const interfaces = os.networkInterfaces();
      let candidates = [];

      // Collect all valid IPv4 addresses
      for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
          if (iface.family === 'IPv4' && !iface.internal) {
            candidates.push({ name, address: iface.address });
          }
        }
      }

      console.log('Detected Network Interfaces:', candidates);

      // Filter 1: Prefer 192.168.x.x BUT exclude Windows Hotspot (192.168.137.x) and VirtualBox (192.168.56.x)
      const wifiIP = candidates.find(c => 
        c.address.startsWith('192.168.') && 
        !c.address.startsWith('192.168.137.') && // Windows Mobile Hotspot
        !c.address.startsWith('192.168.56.')     // VirtualBox
      );

      if (wifiIP) return wifiIP.address;

      // Filter 2: Fallback to any 192.168.x.x (if we are actually using Hotspot)
      const anyLocal = candidates.find(c => c.address.startsWith('192.168.'));
      if (anyLocal) return anyLocal.address;

      // Filter 3: Any non-internal IP
      return candidates.length > 0 ? candidates[0].address : '0.0.0.0';
    };

    const localIP = getLocalIP();
    // Bind to the specific interface to ensure broadcast goes to the right network
    const bonjour = require('bonjour')({ interface: localIP });
    bonjour.publish({
      name: 'esp32-server',
      type: 'http',
      port: PORT,
      txt: {
        path: '/api'
      }
    });
    console.log(`mDNS: Server discoverable at esp32-server.local:${PORT} (Interface: ${localIP})`);
    
    // MQTT Discovery: Publish Server IP to MQTT Broker (Retained)
    // This allows ESP32 to find the server even if mDNS fails
    setTimeout(() => {
      if (mqttService.isConnected()) {
        const discoveryData = JSON.stringify({
          ip: localIP,
          port: PORT,
          url: `http://${localIP}:${PORT}`
        });
        mqttService.client.publish('camera/server-ip', discoveryData, { retain: true });
        console.log(`MQTT Discovery: Published Server IP [${localIP}] to topic 'camera/server-ip'`);
      }
    }, 3000); // Wait a bit for MQTT to connect

  } catch (error) {
    console.log(`mDNS not available (optional feature)`);
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
