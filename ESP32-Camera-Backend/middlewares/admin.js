const User = require('../models/User');

const admin = async (req, res, next) => {
  try {
    // req.user is already set by the auth middleware
    if (!req.user) {
      return res.status(401).json({
        success: false,
        message: 'Authentication required'
      });
    }

    // Check role
    if (req.user.role !== 'admin' && req.user.role !== 'manager') {
      return res.status(403).json({
        success: false,
        message: 'Access denied. Admin or Manager privileges required.'
      });
    }

    next();
  } catch (error) {
    console.error('Admin middleware error:', error);
    res.status(500).json({
      success: false,
      message: 'Server error'
    });
  }
};

exports.admin = admin;
