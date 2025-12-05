/**
 * MQTT Service for ESP32 Camera System
 * Handles MQTT broker connection and message handling
 */

const mqtt = require('mqtt');
const fs = require('fs').promises;
const path = require('path');
const Image = require('../models/Image');
const User = require('../models/User');

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
   */
  async handleImageUpload(message) {
    try {
      const payload = JSON.parse(message.toString());
      console.log('📸 Processing image upload via MQTT...');

      const { userId, imageData, timestamp, detectedObject } = payload;

      // Validate payload
      if (!userId || !imageData) {
        console.error('❌ Invalid image upload payload');
        return;
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
        userId: userId
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
      const user = await User.findById(userId);
      if (user) {
        // Send notification via MQTT
        this.publish(this.topics.notification, JSON.stringify({
          type: 'image_uploaded',
          filename,
          detectedObject: detectedObject || 'unknown',
          timestamp: new Date().toISOString()
        }));
      }

    } catch (error) {
      console.error('❌ Error processing image upload:', error.message);
    }
  }

  /**
   * Handle ESP32 status updates
   */
  async handleStatusUpdate(message) {
    try {
      const status = JSON.parse(message.toString());
      console.log('📊 ESP32 Status:', status);

      // Broadcast status to frontend
      if (this.io) {
        this.io.emit('esp32-status', status);
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
