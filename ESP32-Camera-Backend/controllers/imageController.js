const Image = require('../models/Image');
const User = require('../models/User');
// const cv = require('opencv4nodejs'); // Temporarily disabled - requires native build
const nodemailer = require('nodemailer');
const TelegramBot = require('node-telegram-bot-api');
const path = require('path');
const fs = require('fs');

// Initialize Telegram Bot
let telegramBot = null;
if (process.env.TELEGRAM_TOKEN) {
  telegramBot = new TelegramBot(process.env.TELEGRAM_TOKEN, { polling: false });
}

// Email transporter
const createEmailTransporter = () => {
  if (!process.env.GMAIL_USER || !process.env.GMAIL_PASS) {
    console.warn('Email credentials not configured');
    return null;
  }

  return nodemailer.createTransport({
    service: 'gmail',
    auth: {
      user: process.env.GMAIL_USER,
      pass: process.env.GMAIL_PASS
    }
  });
};

// Detect person using OpenCV (simplified - using face detection as proxy)
const detectPerson = async (imagePath) => {
  try {
    // TODO: Implement actual detection when opencv4nodejs is properly installed
    // For now, always return true to allow testing
    console.log('⚠️  OpenCV detection disabled - assuming person detected');
    console.log('   To enable: install OpenCV libs and rebuild opencv4nodejs');
    return true;
    
    /* Original OpenCV implementation:
    const img = await cv.imreadAsync(imagePath);
    const classifier = new cv.CascadeClassifier(cv.HAAR_FRONTALFACE_ALT2);
    const gray = await img.bgrToGrayAsync();
    const faces = await classifier.detectMultiScaleAsync(gray);
    return faces.objects.length > 0;
    */
  } catch (error) {
    console.error('Detection error:', error.message);
    return true; // Fallback: assume person detected
  }
};

// Send email notification
const sendEmailNotification = async (user, imageData) => {
  try {
    const transporter = createEmailTransporter();
    if (!transporter) {
      console.log('Email transporter not configured, skipping email notification');
      return;
    }

    const mailOptions = {
      from: process.env.GMAIL_USER,
      to: user.email,
      subject: '🚨 Security Alert: Person Detected',
      html: `
        <h2>Security Camera Alert</h2>
        <p>A person has been detected by your ESP32 security camera.</p>
        <p><strong>Detection Time:</strong> ${new Date(imageData.timestamp).toLocaleString()}</p>
        <p><strong>Detected Object:</strong> ${imageData.detectedObject}</p>
        <p>Please check the attached image for details.</p>
      `,
      attachments: [
        {
          filename: imageData.filename,
          path: imageData.path
        }
      ]
    };

    await transporter.sendMail(mailOptions);
    console.log(`Email sent to ${user.email}`);
  } catch (error) {
    console.error('Email sending error:', error.message);
  }
};

// Send Telegram notification
const sendTelegramNotification = async (user, imageData) => {
  try {
    if (!telegramBot || !user.telegramId) {
      console.log('Telegram not configured or user has no Telegram ID');
      return;
    }

    const message = `🚨 *Security Alert*\n\n` +
      `Person detected by your ESP32 security camera\n` +
      `*Time:* ${new Date(imageData.timestamp).toLocaleString()}\n` +
      `*Detected:* ${imageData.detectedObject}`;

    await telegramBot.sendPhoto(user.telegramId, imageData.path, {
      caption: message,
      parse_mode: 'Markdown'
    });

    console.log(`Telegram notification sent to ${user.telegramId}`);
  } catch (error) {
    console.error('Telegram sending error:', error.message);
  }
};

// @desc    Upload image from ESP32 and process
// @route   POST /api/upload-image
// @access  Private (JWT)
exports.uploadImage = async (req, res) => {
  try {
    if (!req.file) {
      return res.status(400).json({
        success: false,
        message: 'No image file provided'
      });
    }

    const imagePath = req.file.path;
    const filename = req.file.filename;

    console.log(`Image received: ${filename}`);

    // Normalize path for URL (convert backslashes to forward slashes)
    // Remove any leading slashes and ensure it starts with /uploads/
    let normalizedPath = imagePath.replace(/\\/g, '/');
    
    // If path starts with /app/ (Docker) or absolute path, extract just the uploads/... part
    if (normalizedPath.includes('/uploads/')) {
      normalizedPath = '/uploads/' + normalizedPath.split('/uploads/')[1];
    } else if (!normalizedPath.startsWith('/uploads/')) {
      // Ensure path starts with /uploads/
      normalizedPath = '/uploads/' + filename;
    }
    
    console.log(`Normalized path: ${normalizedPath}`);

    // Detect person using OpenCV
    const isPersonDetected = await detectPerson(imagePath);

    if (isPersonDetected) {
      console.log('Person detected in image!');

      // Save to MongoDB
      const image = await Image.create({
        filename,
        path: normalizedPath,  // Use normalized path for URL
        timestamp: new Date(),
        detectedObject: 'person',
        userId: req.user._id
      });

      // Get user data for notifications
      const user = await User.findById(req.user._id);

      // Send notifications in parallel
      await Promise.all([
        sendEmailNotification(user, {
          filename,
          path: imagePath,
          timestamp: image.timestamp,
          detectedObject: 'person'
        }),
        sendTelegramNotification(user, {
          filename,
          path: imagePath,
          timestamp: image.timestamp,
          detectedObject: 'person'
        })
      ]);

      res.status(201).json({
        success: true,
        message: 'Person detected! Image saved and notifications sent.',
        data: {
          image: {
            id: image._id,
            filename: image.filename,
            timestamp: image.timestamp,
            detectedObject: image.detectedObject
          }
        }
      });
    } else {
      // No person detected, delete image
      fs.unlinkSync(imagePath);
      console.log('No person detected, image deleted');

      res.status(200).json({
        success: false,
        message: 'No person detected in image'
      });
    }
  } catch (error) {
    console.error('Upload error:', error.message);
    
    // Clean up file on error
    if (req.file && req.file.path) {
      try {
        fs.unlinkSync(req.file.path);
      } catch (e) {
        console.error('Error deleting file:', e.message);
      }
    }

    res.status(500).json({
      success: false,
      message: 'Error processing image',
      error: error.message
    });
  }
};

// @desc    Get all images with pagination
// @route   GET /api/images
// @access  Private (JWT)
exports.getImages = async (req, res) => {
  try {
    const page = parseInt(req.query.page) || 1;
    const limit = parseInt(req.query.limit) || 10;
    const skip = (page - 1) * limit;

    // Get images for current user
    const images = await Image.find({ userId: req.user._id })
      .sort({ timestamp: -1 })
      .skip(skip)
      .limit(limit)
      .select('-__v');

    const total = await Image.countDocuments({ userId: req.user._id });

    res.status(200).json({
      success: true,
      data: {
        images,
        pagination: {
          current: page,
          total: Math.ceil(total / limit),
          count: images.length,
          totalImages: total
        }
      }
    });
  } catch (error) {
    console.error('Get images error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error fetching images',
      error: error.message
    });
  }
};

// @desc    Get single image by ID
// @route   GET /api/images/:id
// @access  Private (JWT)
exports.getImageById = async (req, res) => {
  try {
    const image = await Image.findOne({
      _id: req.params.id,
      userId: req.user._id
    });

    if (!image) {
      return res.status(404).json({
        success: false,
        message: 'Image not found'
      });
    }

    res.status(200).json({
      success: true,
      data: image
    });
  } catch (error) {
    console.error('Get image error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error fetching image',
      error: error.message
    });
  }
};

// @desc    Delete image
// @route   DELETE /api/images/:id
// @access  Private (JWT)
exports.deleteImage = async (req, res) => {
  try {
    const image = await Image.findOne({
      _id: req.params.id,
      userId: req.user._id
    });

    if (!image) {
      return res.status(404).json({
        success: false,
        message: 'Image not found'
      });
    }

    // Delete file from filesystem (resolve absolute path)
    try {
      const relPath = typeof image.path === 'string' ? image.path.replace(/^\/+/, '') : '';
      const absolutePath = path.isAbsolute(image.path)
        ? image.path
        : path.join(__dirname, '..', relPath);
      if (fs.existsSync(absolutePath)) {
        fs.unlinkSync(absolutePath);
      }
    } catch (e) {
      console.error('Error deleting image file from disk:', e.message);
    }

    // Delete from database
    await Image.deleteOne({ _id: req.params.id });

    res.status(200).json({
      success: true,
      message: 'Image deleted successfully'
    });
  } catch (error) {
    console.error('Delete image error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error deleting image',
      error: error.message
    });
  }
};

// @desc    Update user notification settings
// @route   PUT /api/config
// @access  Private (JWT)
exports.updateConfig = async (req, res) => {
  try {
    const { email, telegramId } = req.body;
    
    const updateData = {};
    if (email) updateData.email = email;
    if (telegramId !== undefined) updateData.telegramId = telegramId;

    const user = await User.findByIdAndUpdate(
      req.user._id,
      updateData,
      { new: true, runValidators: true }
    ).select('-password');

    res.status(200).json({
      success: true,
      message: 'Configuration updated successfully',
      data: user
    });
  } catch (error) {
    console.error('Update config error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error updating configuration',
      error: error.message
    });
  }
};

// @desc    Check for new images since last check
// @route   GET /api/images/check-new?lastCheckTime=timestamp
// @access  Private
exports.checkNewImages = async (req, res) => {
  try {
    const { lastCheckTime } = req.query;
    
    if (!lastCheckTime) {
      return res.status(400).json({
        success: false,
        message: 'lastCheckTime parameter is required'
      });
    }

    // Convert timestamp to Date
    const lastCheck = new Date(parseInt(lastCheckTime));

    // Count new images since last check for current user
    const newImagesCount = await Image.countDocuments({
      userId: req.user._id,
      timestamp: { $gt: lastCheck }
    });

    // Get the latest image for preview (current user)
    const latestImage = newImagesCount > 0 
      ? await Image.findOne({ userId: req.user._id }).sort({ timestamp: -1 })
      : null;

    res.status(200).json({
      success: true,
      data: {
        hasNewImages: newImagesCount > 0,
        newImagesCount,
        latestImage,
        checkTime: new Date().getTime()
      }
    });
  } catch (error) {
    console.error('Check new images error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error checking for new images',
      error: error.message
    });
  }
};
