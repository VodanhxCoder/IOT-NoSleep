/**
 * MQTT Service for ESP32 Camera System
 * Handles MQTT broker connection and message handling
 */

const mqtt = require('mqtt');
const fs = require('fs').promises;
const path = require('path');
const mongoose = require('mongoose');
const Image = require('../models/Image');
const User = require('../models/User');
const notificationService = require('./notificationService');

class MQTTService {
  constructor() {
    this.client = null;
    this.io = null; // Socket.IO instance for real-time updates
    
    // MQTT Configuration
    this.config = {
      broker: process.env.MQTT_BROKER || 'mqtt://localhost:1883',
      clientId: 'esp32-backend-' + Math.random().toString(16).substr(2, 8),
      username: process.env.MQTT_USERNAME || '',
      password: process.env.MQTT_PASSWORD || '',
      reconnectPeriod: 5000,
      connectTimeout: 30000,
    };

    // Topics
    this.topics = {
      imageUpload: 'esp32/camera/image',
      status: 'esp32/camera/status',
      command: 'esp32/camera/command',
      notification: 'esp32/camera/notification'
    };

    // Buffer for chunked image uploads
    this.chunkBuffer = new Map();

    // Store last known status
    this.lastStatus = { status: 'unknown', ip: 'unknown' };
    
    // Load saved state from disk
    this.loadSavedState();
  }

  /**
   * Load saved ESP32 state from disk
   */
  async loadSavedState() {
    try {
      const configPath = path.join(__dirname, '../config/esp32_state.json');
      const data = await fs.readFile(configPath, 'utf8');
      const savedState = JSON.parse(data);
      
      if (savedState.streamUrl) {
        console.log('💾 Loaded saved ESP32 configuration:', savedState);
        process.env.ESP32_STREAM_URL = savedState.streamUrl;
        this.lastStatus = { ...this.lastStatus, ...savedState };
      }
    } catch (error) {
      // Ignore error if file doesn't exist (first run)
      if (error.code !== 'ENOENT') {
        console.error('⚠️ Failed to load saved ESP32 state:', error.message);
      }
    }
  }

  /**
   * Save ESP32 state to disk
   */
  async saveState(state) {
    try {
      const configPath = path.join(__dirname, '../config/esp32_state.json');
      // Ensure config directory exists
      await fs.mkdir(path.dirname(configPath), { recursive: true });
      await fs.writeFile(configPath, JSON.stringify(state, null, 2));
    } catch (error) {
      console.error('❌ Failed to save ESP32 state:', error.message);
    }
  }

  /**
   * Initialize MQTT connection
   */
  async connect() {
    try {
      console.log('🔌 Connecting to MQTT broker:', this.config.broker);
      
      const options = {
        clientId: this.config.clientId,
        reconnectPeriod: this.config.reconnectPeriod,
        connectTimeout: this.config.connectTimeout,
        clean: true,
      };

      // Add auth if credentials provided
      if (this.config.username) {
        options.username = this.config.username;
        options.password = this.config.password;
      }

      this.client = mqtt.connect(this.config.broker, options);

      // Connection handlers
      this.client.on('connect', () => {
        console.log('✅ MQTT Connected successfully');
        console.log('📡 MQTT Client ID:', this.config.clientId);
        
        // Subscribe to topics
        this.subscribeToTopics();
      });

      this.client.on('error', (error) => {
        console.error('❌ MQTT Connection error:', error.message);
      });

      this.client.on('reconnect', () => {
        console.log('🔄 MQTT Reconnecting...');
      });

      this.client.on('offline', () => {
        console.log('📴 MQTT Client offline');
      });

      this.client.on('message', async (topic, message) => {
        await this.handleMessage(topic, message);
      });

    } catch (error) {
      console.error('❌ MQTT Connection failed:', error.message);
    }
  }

  /**
   * Subscribe to MQTT topics
   */
  subscribeToTopics() {
    const topics = Object.values(this.topics);
    
    topics.forEach(topic => {
      this.client.subscribe(topic, (err) => {
        if (err) {
          console.error(`❌ Failed to subscribe to ${topic}:`, err.message);
        } else {
          console.log(`📬 Subscribed to topic: ${topic}`);
        }
      });
    });
  }

  /**
   * Handle incoming MQTT messages
   */
  async handleMessage(topic, message) {
    try {
      console.log(`📨 MQTT Message received on topic: ${topic}`);

      switch (topic) {
        case this.topics.imageUpload:
          await this.handleImageUpload(message);
          break;

        case this.topics.status:
          await this.handleStatusUpdate(message);
          break;

        case this.topics.notification:
          await this.handleNotification(message);
          break;

        default:
          console.log(`⚠️  Unknown topic: ${topic}`);
      }
    } catch (error) {
      console.error('❌ Error handling MQTT message:', error.message);
    }
  }

  /**
   * Handle image upload from ESP32
   * Payload format: JSON with { userId, imageData (base64), timestamp }
   * OR Chunked format: { id, index, total, data, userId }
   */
  async handleImageUpload(message) {
    try {
      const payload = JSON.parse(message.toString());
      
      // Check if this is a chunked upload
      if (payload.index !== undefined && payload.total !== undefined && payload.id) {
        await this.handleChunk(payload);
        return;
      }

      console.log('📸 Processing image upload via MQTT...');

      const { userId, imageData, timestamp, detectedObject } = payload;

      // Validate payload
      if (!userId || !imageData) {
        console.error('❌ Invalid image upload payload');
        return;
      }

      // Process the complete image
      await this.processCompleteImage({
        userId,
        imageData,
        timestamp,
        detectedObject
      });

    } catch (error) {
      console.error('❌ Error processing image upload:', error.message);
    }
  }

  /**
   * Handle chunked image upload
   */
  async handleChunk(payload) {
    const { id, index, total, data, userId } = payload;
    
    // 1. Initialize buffer if new
    if (!this.chunkBuffer.has(id)) {
      this.chunkBuffer.set(id, {
        chunks: new Array(total).fill(null),
        receivedCount: 0,
        timestamp: Date.now(),
        userId: userId // Save info from first received chunk
      });
      
      // Set timeout to clean up incomplete uploads
      setTimeout(() => {
        if (this.chunkBuffer.has(id)) {
          console.log(`🗑️ Timeout: Dropped incomplete image ${id}`);
          this.chunkBuffer.delete(id);
        }
      }, 60000); // 60s timeout
    }

    const bufferEntry = this.chunkBuffer.get(id);

    // 2. Store chunk
    if (bufferEntry.chunks[index] === null) {
      bufferEntry.chunks[index] = data;
      bufferEntry.receivedCount++;
    }

    console.log(`🧩 Received chunk ${index + 1}/${total} for image ${id}`);

    // 3. Check if complete
    if (bufferEntry.receivedCount === total) {
      console.log(`🎉 Image ${id} reassembled successfully!`);
      
      // Reassemble full base64 string
      const fullBase64 = bufferEntry.chunks.join('');
      
      // Process complete image
      await this.processCompleteImage({
        userId: bufferEntry.userId,
        imageData: fullBase64,
        timestamp: new Date().toISOString(),
        detectedObject: 'unknown' // Default for now
      });
      
      // Cleanup
      this.chunkBuffer.delete(id);
    }
  }

  /**
   * Process complete image data (save to disk/DB)
   */
  async processCompleteImage({ userId, imageData, timestamp, detectedObject }) {
    try {
      // Resolve userId if it's a username (string) instead of ObjectId
      let resolvedUserId = userId;
      if (userId && !mongoose.Types.ObjectId.isValid(userId)) {
        console.log(`🔍 Looking up user by username: ${userId}`);
        const user = await User.findOne({ username: userId });
        if (user) {
          resolvedUserId = user._id;
          console.log(`✅ Resolved username '${userId}' to ID: ${resolvedUserId}`);
        } else {
          console.error(`❌ User not found for username: ${userId}`);
          // Optional: Create a default user or fail? 
          // For now, let's fail gracefully but maybe we should check if there's ANY user to fallback to?
          // Let's try to find the first user as fallback if specific user not found (for robustness)
          const firstUser = await User.findOne();
          if (firstUser) {
             console.log(`⚠️ Using fallback user: ${firstUser.username}`);
             resolvedUserId = firstUser._id;
          } else {
             throw new Error(`User '${userId}' not found and no users exist in DB`);
          }
        }
      }

      // Decode base64 image
      const imageBuffer = Buffer.from(imageData, 'base64');
      
      // Generate filename
      const filename = `capture-${Date.now()}-${Math.floor(Math.random() * 1000000000)}.jpg`;
      const uploadDir = path.join(__dirname, '../uploads');
      const imagePath = path.join(uploadDir, filename);

      // Save image to disk
      await fs.mkdir(uploadDir, { recursive: true });
      await fs.writeFile(imagePath, imageBuffer);

      console.log(`✅ Image saved: ${filename}`);

      // Save to MongoDB
      const image = await Image.create({
        filename,
        path: '/uploads/' + filename,
        timestamp: timestamp ? new Date(timestamp) : new Date(),
        detectedObject: detectedObject || 'unknown',
        userId: resolvedUserId
      });

      console.log(`✅ Image record created in database`);

      // Send real-time update to frontend via Socket.IO
      if (this.io) {
        this.io.emit('new-image', {
          id: image._id,
          filename: image.filename,
          path: image.path,
          timestamp: image.timestamp,
          detectedObject: image.detectedObject
        });
        console.log('📡 Real-time update sent to frontend');
      }

      // Get user for notifications
      const user = await User.findById(resolvedUserId);
      if (user) {
        // Send notification via MQTT
        this.publish(this.topics.notification, JSON.stringify({
          type: 'image_uploaded',
          filename,
          detectedObject: detectedObject || 'unknown',
          timestamp: new Date().toISOString()
        }));

        // Send Email & Telegram Notifications
        // We pass the absolute path for attachments
        const notificationData = {
            filename,
            path: imagePath, // Absolute path on disk
            timestamp: timestamp ? new Date(timestamp) : new Date(),
            detectedObject: detectedObject || 'unknown'
        };

        // Run in background
        notificationService.sendEmail(user, notificationData);
        notificationService.sendTelegram(user, notificationData);
      }
    } catch (error) {
      console.error('❌ Error saving reassembled image:', error.message);
    }
  }

  /**
   * Handle ESP32 status updates
   */
  async handleStatusUpdate(message) {
    try {
      // Check if message is JSON or plain text
      let status;
      try {
        status = JSON.parse(message.toString());
      } catch (e) {
        // If not JSON, treat as plain text status string (e.g. "online", "offline")
        status = { status: message.toString() };
      }
      
      console.log('📊 ESP32 Status:', status);

      // Update Stream URL if provided by ESP32
      if (status.streamUrl) {
        console.log(`🎥 Auto-configuring Stream URL: ${status.streamUrl}`);
        process.env.ESP32_STREAM_URL = status.streamUrl;
        
        // Save to disk for persistence
        await this.saveState({
            streamUrl: status.streamUrl,
            ip: status.ip,
            status: status.status,
            lastUpdated: new Date().toISOString()
        });
      }

      // Update last known status
      this.lastStatus = { ...this.lastStatus, ...status, lastSeen: new Date() };

      // Broadcast status to frontend
      if (this.io) {
        this.io.emit('esp32-status', this.lastStatus);
      }
    } catch (error) {
      console.error('❌ Error handling status update:', error.message);
    }
  }

  /**
   * Handle notifications
   */
  async handleNotification(message) {
    try {
      const notification = JSON.parse(message.toString());
      console.log('🔔 Notification:', notification);

      // Broadcast to frontend
      if (this.io) {
        this.io.emit('notification', notification);
      }
    } catch (error) {
      console.error('❌ Error handling notification:', error.message);
    }
  }

  /**
   * Publish message to MQTT topic
   */
  publish(topic, message, options = {}) {
    if (!this.client || !this.client.connected) {
      console.error('❌ MQTT client not connected');
      return false;
    }

    this.client.publish(topic, message, { qos: 1, ...options }, (err) => {
      if (err) {
        console.error(`❌ Failed to publish to ${topic}:`, err.message);
      } else {
        console.log(`✅ Published to ${topic}`);
      }
    });

    return true;
  }

  /**
   * Send command to ESP32
   */
  sendCommand(command) {
    return this.publish(this.topics.command, JSON.stringify(command));
  }

  /**
   * Get last known status
   */
  getLastStatus() {
    return this.lastStatus;
  }

  /**
   * Set Socket.IO instance for real-time updates
   */
  setSocketIO(io) {
    this.io = io;
    console.log('🔌 Socket.IO instance set for MQTT service');
  }

  /**
   * Disconnect from MQTT broker
   */
  disconnect() {
    if (this.client) {
      this.client.end();
      console.log('🔌 MQTT Disconnected');
    }
  }

  /**
   * Get connection status
   */
  isConnected() {
    return this.client && this.client.connected;
  }
}

// Export singleton instance
module.exports = new MQTTService();
