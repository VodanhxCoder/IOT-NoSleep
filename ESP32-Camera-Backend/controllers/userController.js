const User = require('../models/User');
const mqttService = require('../services/mqttService');


// @desc    Get all users
// @route   GET /api/users
// @access  Admin
exports.getUsers = async (req, res) => {
  try {
    const users = await User.find().select('-password').sort({ createdAt: -1 });
    res.status(200).json({
      success: true,
      count: users.length,
      data: users
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: error.message
    });
  }
};

// @desc    Create new user (Admin)
// @route   POST /api/users
// @access  Admin
exports.createUser = async (req, res) => {
  try {
    const { username, email, password, role } = req.body;

    // Check if user exists
    const userExists = await User.findOne({ $or: [{ email }, { username }] });
    if (userExists) {
      return res.status(400).json({
        success: false,
        message: 'User already exists'
      });
    }

    // Create user
    const user = await User.create({
      username,
      email,
      password,
      role: role || 'user'
    });

    res.status(201).json({
      success: true,
      data: {
        _id: user._id,
        username: user.username,
        email: user.email,
        role: user.role,
        createdAt: user.createdAt
      }
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: error.message
    });
  }
};

// @desc    Ban user
// @route   PUT /api/users/:id/ban
// @access  Admin
exports.banUser = async (req, res) => {
  try {
    const user = await User.findById(req.params.id);

    if (!user) {
      return res.status(404).json({
        success: false,
        message: 'User not found'
      });
    }

    // Prevent banning self
    if (user._id.toString() === req.user.id) {
      return res.status(400).json({
        success: false,
        message: 'Cannot ban yourself'
      });
    }

    // Hierarchy check: Managers cannot ban Admins
    if (req.user.role === 'manager' && user.role === 'admin') {
      return res.status(403).json({
        success: false,
        message: 'Managers cannot ban Admins'
      });
    }

    user.isBanned = true;
    await user.save();

    // Emit socket event to force logout if online
    const io = req.app.get('io');
    if (io) {
      io.emit('user-banned', { userId: user._id });
    }

    // Notify ESP32 via MQTT (so it can log it)
    if (mqttService.isConnected()) {
      mqttService.publish(mqttService.topics.command, `User ${user.username} has been banned`);
    }

    res.status(200).json({
      success: true,
      message: `User ${user.username} has been banned`,
      data: user
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: error.message
    });
  }
};

// @desc    Unban user
// @route   PUT /api/users/:id/unban
// @access  Admin
exports.unbanUser = async (req, res) => {
  try {
    const user = await User.findById(req.params.id);

    if (!user) {
      return res.status(404).json({
        success: false,
        message: 'User not found'
      });
    }

    // Hierarchy check
    if (req.user.role === 'manager' && user.role === 'admin') {
      return res.status(403).json({
        success: false,
        message: 'Managers cannot unban Admins'
      });
    }

    user.isBanned = false;
    await user.save();

    res.status(200).json({
      success: true,
      message: `User ${user.username} has been unbanned`,
      data: user
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: error.message
    });
  }
};

// @desc    Delete user
// @route   DELETE /api/users/:id
// @access  Admin
exports.deleteUser = async (req, res) => {
  try {
    const user = await User.findById(req.params.id);

    if (!user) {
      return res.status(404).json({
        success: false,
        message: 'User not found'
      });
    }

    // Prevent deleting self
    if (user._id.toString() === req.user.id) {
      return res.status(400).json({
        success: false,
        message: 'Cannot delete yourself'
      });
    }

    // Hierarchy check
    if (req.user.role === 'manager' && user.role === 'admin') {
      return res.status(403).json({
        success: false,
        message: 'Managers cannot delete Admins'
      });
    }

    await user.deleteOne();

    res.status(200).json({
      success: true,
      message: `User ${user.username} has been deleted`
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: error.message
    });
  }
};

// @desc    Get online users (Socket.IO based)
// @route   GET /api/users/online
// @access  Admin
exports.getOnlineUsers = async (req, res) => {
  try {
    // This requires tracking socket connections mapped to user IDs
    // For now, we can return users active in the last 5 minutes
    const fiveMinutesAgo = new Date(Date.now() - 5 * 60 * 1000);
    
    const onlineUsers = await User.find({
      lastActive: { $gte: fiveMinutesAgo }
    }).select('username email role lastActive');

    res.status(200).json({
      success: true,
      count: onlineUsers.length,
      data: onlineUsers
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: error.message
    });
  }
};
