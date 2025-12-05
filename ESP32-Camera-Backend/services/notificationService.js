const nodemailer = require('nodemailer');
const TelegramBot = require('node-telegram-bot-api');
const path = require('path');

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

class NotificationService {
  /**
   * Send email notification to user(s)
   * @param {Object} user User object with email and notificationEmails
   * @param {Object} imageData Image data object
   */
  async sendEmail(user, imageData) {
    try {
      // Check if user is banned
      if (user.isBanned) {
        console.log('🚫 User is banned, skipping email notification');
        return;
      }

      const transporter = createEmailTransporter();
      if (!transporter) {
        console.log('Email transporter not configured, skipping email notification');
        return;
      }

      // Collect all recipients
      let recipients = [];
      
      // 1. Add primary email if enabled
      if (user.notifyEmail && user.email) {
        recipients.push(user.email);
      }

      // 2. Add additional emails if active
      if (user.notificationEmails && Array.isArray(user.notificationEmails)) {
        const activeEmails = user.notificationEmails
          .filter(e => e.active)
          .map(e => e.address);
        recipients = [...recipients, ...activeEmails];
      }

      // Remove duplicates
      recipients = [...new Set(recipients)];

      if (recipients.length === 0) {
        console.log('No active email recipients found');
        return;
      }

      console.log(`📧 Sending email to ${recipients.length} recipients: ${recipients.join(', ')}`);

      const mailOptions = {
        from: process.env.GMAIL_USER,
        to: recipients.join(', '), // Send to all at once
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
      console.log(`✅ Email sent successfully`);
    } catch (error) {
      console.error('❌ Email sending error:', error.message);
    }
  }

  /**
   * Send Telegram notification
   * @param {Object} user User object
   * @param {Object} imageData Image data object
   */
  async sendTelegram(user, imageData) {
    try {
      // Check if user is banned
      if (user.isBanned) {
        console.log('🚫 User is banned, skipping Telegram notification');
        return;
      }

      if (!user.notifyTelegram) {
        console.log('Telegram notification disabled for user');
        return;
      }

      if (!telegramBot || !user.telegramId) {
        console.log('Telegram not configured or user has no Telegram ID');
        return;
      }

      const message = `🚨 *Security Alert*\n\n` +
        `Person detected by your ESP32 security camera\n` +
        `*Time:* ${new Date(imageData.timestamp).toLocaleString()}\n` +
        `*Detected:* ${imageData.detectedObject}`;

      // Ensure path is absolute
      let imagePath = imageData.path;
      if (!path.isAbsolute(imagePath)) {
         // If it's a relative path like '/uploads/file.jpg', resolve it
         if (imagePath.startsWith('/uploads/')) {
             imagePath = path.join(__dirname, '..', imagePath);
         } else {
             // Fallback
             imagePath = path.join(__dirname, '../uploads', imageData.filename);
         }
      }

      await telegramBot.sendPhoto(user.telegramId, imagePath, {
        caption: message,
        parse_mode: 'Markdown'
      });

      console.log(`✅ Telegram notification sent to ${user.telegramId}`);
    } catch (error) {
      console.error('❌ Telegram sending error:', error.message);
    }
  }
}

module.exports = new NotificationService();
