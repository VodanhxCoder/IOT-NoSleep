import React, { useState, useEffect } from 'react';
import { Save, Mail, MessageSquare, Bell, User } from 'lucide-react';
import imageService from '../services/imageService';
import { useAuth } from '../context/AuthContext';

const Settings = () => {
  const { user } = useAuth();
  const [formData, setFormData] = useState({
    email: '',
    telegramId: '',
  });
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState({ type: '', text: '' });

  useEffect(() => {
    if (user) {
      setFormData({
        email: user.email || '',
        telegramId: user.telegramId || '',
      });
    }
  }, [user]);

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setMessage({ type: '', text: '' });

    try {
      await imageService.updateConfig(formData);
      setMessage({ type: 'success', text: 'Settings updated successfully!' });
    } catch (error) {
      setMessage({ 
        type: 'error', 
        text: error.response?.data?.message || 'Failed to update settings' 
      });
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900">
      <div className="max-w-4xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8">
          <h1 className="text-3xl font-bold text-gray-900 dark:text-white">Settings</h1>
          <p className="text-gray-600 dark:text-gray-400 mt-2">Manage your notification preferences</p>
        </div>

        <div className="card">
          <form onSubmit={handleSubmit} className="space-y-6">
            {/* User Info */}
            <div className="pb-6 border-b border-gray-200 dark:border-gray-700">
              <h2 className="text-xl font-semibold text-gray-900 dark:text-white mb-4 flex items-center">
                <User className="w-5 h-5 mr-2" />
                Account Information
              </h2>
              <div className="space-y-3 text-gray-600 dark:text-gray-400">
                <p><strong>Username:</strong> {user?.username}</p>
                <p><strong>Email:</strong> {user?.email || 'Not set'}</p>
              </div>
            </div>

            {/* Email Notifications */}
            <div>
              <h2 className="text-xl font-semibold text-gray-900 dark:text-white mb-4 flex items-center">
                <Mail className="w-5 h-5 mr-2" />
                Email Notifications
              </h2>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  Email Address
                </label>
                <input
                  type="email"
                  value={formData.email}
                  onChange={(e) => setFormData({ ...formData, email: e.target.value })}
                  className="input"
                  placeholder="your@email.com"
                />
                <p className="mt-2 text-sm text-gray-500 dark:text-gray-400">
                  Receive email alerts when a person is detected by the camera
                </p>
              </div>
            </div>

            {/* Telegram Notifications */}
            <div>
              <h2 className="text-xl font-semibold text-gray-900 dark:text-white mb-4 flex items-center">
                <MessageSquare className="w-5 h-5 mr-2" />
                Telegram Notifications
              </h2>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                  Telegram Chat ID
                </label>
                <input
                  type="text"
                  value={formData.telegramId}
                  onChange={(e) => setFormData({ ...formData, telegramId: e.target.value })}
                  className="input"
                  placeholder="123456789"
                />
                <p className="mt-2 text-sm text-gray-500 dark:text-gray-400">
                  Get instant Telegram messages when motion is detected
                </p>
                <div className="mt-3 p-3 bg-blue-50 dark:bg-blue-900 rounded-lg">
                  <p className="text-sm text-blue-800 dark:text-blue-200">
                    <strong>How to get your Chat ID:</strong>
                  </p>
                  <ol className="mt-2 text-sm text-blue-700 dark:text-blue-300 list-decimal list-inside space-y-1">
                    <li>Open Telegram and search for @userinfobot</li>
                    <li>Start a chat and send /start</li>
                    <li>Copy the Chat ID provided</li>
                    <li>Make sure you've also started a chat with your configured bot</li>
                  </ol>
                </div>
              </div>
            </div>

            {/* Message */}
            {message.text && (
              <div className={`p-4 rounded-lg ${
                message.type === 'success' 
                  ? 'bg-green-100 dark:bg-green-900 text-green-800 dark:text-green-200'
                  : 'bg-red-100 dark:bg-red-900 text-red-800 dark:text-red-200'
              }`}>
                {message.text}
              </div>
            )}

            {/* Submit Button */}
            <div className="pt-6 border-t border-gray-200 dark:border-gray-700">
              <button
                type="submit"
                disabled={loading}
                className="btn-primary w-full sm:w-auto flex items-center justify-center space-x-2 disabled:opacity-50"
              >
                <Save className="w-5 h-5" />
                <span>{loading ? 'Saving...' : 'Save Settings'}</span>
              </button>
            </div>
          </form>
        </div>

        {/* Info Card */}
        <div className="mt-6 card bg-primary-50 dark:bg-primary-900">
          <div className="flex items-start space-x-3">
            <Bell className="w-6 h-6 text-primary-600 dark:text-primary-400 flex-shrink-0 mt-1" />
            <div>
              <h3 className="font-semibold text-gray-900 dark:text-white mb-2">
                Notification System
              </h3>
              <p className="text-sm text-gray-700 dark:text-gray-300">
                Configure your email and Telegram to receive real-time alerts when your ESP32 camera detects 
                a person. Make sure the backend server has valid SMTP and Telegram bot credentials configured.
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default Settings;
