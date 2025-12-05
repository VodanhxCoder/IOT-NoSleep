const User = require('../models/User');

// @desc    Update general configuration
// @route   PUT /api/config
// @access  Private
exports.updateConfig = async (req, res) => {
  try {
    const user = await User.findById(req.user._id);

    if (!user) {
      return res.status(404).json({
        success: false,
        message: 'User not found'
      });
    }

    const { telegramId, notifyEmail, notifyTelegram } = req.body;

    if (telegramId !== undefined) user.telegramId = telegramId;
    if (notifyEmail !== undefined) user.notifyEmail = notifyEmail;
    if (notifyTelegram !== undefined) user.notifyTelegram = notifyTelegram;

    await user.save();

    res.status(200).json({
      success: true,
      message: 'Configuration updated',
      data: {
        telegramId: user.telegramId,
        notifyEmail: user.notifyEmail,
        notifyTelegram: user.notifyTelegram
      }
    });
  } catch (error) {
    console.error('Update config error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error updating configuration'
    });
  }
};

// @desc    Get notification emails
// @route   GET /api/config/emails
// @access  Private
exports.getEmails = async (req, res) => {
  try {
    const user = await User.findById(req.user._id);
    res.status(200).json({
      success: true,
      data: user.notificationEmails || []
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: 'Error fetching emails'
    });
  }
};

// @desc    Add notification email
// @route   POST /api/config/emails
// @access  Private
exports.addEmail = async (req, res) => {
  try {
    const { address, label, active } = req.body;
    
    if (!address) {
      return res.status(400).json({
        success: false,
        message: 'Email address is required'
      });
    }

    const user = await User.findById(req.user._id);
    
    // Check duplicate
    if (user.notificationEmails.some(e => e.address === address)) {
        return res.status(400).json({
            success: false,
            message: 'Email already exists'
        });
    }

    user.notificationEmails.push({ address, label, active });
    await user.save();

    res.status(201).json({
      success: true,
      message: 'Email added',
      data: user.notificationEmails
    });
  } catch (error) {
    console.error('Add email error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error adding email'
    });
  }
};

// @desc    Update notification email
// @route   PUT /api/config/emails/:id
// @access  Private
exports.updateEmail = async (req, res) => {
  try {
    const { address, label, active } = req.body;
    const user = await User.findById(req.user._id);
    
    const email = user.notificationEmails.id(req.params.id);
    if (!email) {
      return res.status(404).json({
        success: false,
        message: 'Email not found'
      });
    }

    if (address) email.address = address;
    if (label !== undefined) email.label = label;
    if (active !== undefined) email.active = active;

    await user.save();

    res.status(200).json({
      success: true,
      message: 'Email updated',
      data: user.notificationEmails
    });
  } catch (error) {
    res.status(500).json({
      success: false,
      message: 'Error updating email'
    });
  }
};

// @desc    Delete notification email
// @route   DELETE /api/config/emails/:id
// @access  Private
exports.deleteEmail = async (req, res) => {
  try {
    const user = await User.findById(req.user._id);
    
    // Filter out the email to delete
    user.notificationEmails = user.notificationEmails.filter(
        e => e._id.toString() !== req.params.id
    );

    await user.save();

    res.status(200).json({
      success: true,
      message: 'Email deleted',
      data: user.notificationEmails
    });
  } catch (error) {
    console.error('Delete email error:', error.message);
    res.status(500).json({
      success: false,
      message: 'Error deleting email'
    });
  }
};
