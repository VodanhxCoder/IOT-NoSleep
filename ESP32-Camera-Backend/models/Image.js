const mongoose = require('mongoose');

const imageSchema = new mongoose.Schema({
  filename: {
    type: String,
    required: true
  },
  path: {
    type: String,
    required: true
  },
  timestamp: {
    type: Date,
    default: Date.now
  },
  detectedObject: {
    type: String,
    default: 'unknown'
  },
  userId: {
    type: mongoose.Schema.Types.ObjectId,
    ref: 'User',
    required: true
  }
}, {
  timestamps: true
});

// Index for faster queries
imageSchema.index({ timestamp: -1 });
imageSchema.index({ userId: 1, timestamp: -1 });

module.exports = mongoose.model('Image', imageSchema);
