import React, { useState, useEffect, useCallback } from 'react';
import { Save, Mail, MessageSquare, Bell, User, Plus, Edit2, Trash2 } from 'lucide-react';
import imageService from '../services/imageService';
import { useAuth } from '../context/AuthContext';

const Settings = () => {
  const { user, refreshUser } = useAuth();
  const [formData, setFormData] = useState({
    telegramId: '',
    notifyEmail: true,
    notifyTelegram: true,
  });
  const [loading, setLoading] = useState(false);
  const [message, setMessage] = useState({ type: '', text: '' });
  const [emailRecipients, setEmailRecipients] = useState([]);
  const [emailsLoading, setEmailsLoading] = useState(false);
  const [emailForm, setEmailForm] = useState({ address: '', label: '', active: true });
  const [editingEmailId, setEditingEmailId] = useState(null);

  useEffect(() => {
    if (user) {
      setFormData({
        telegramId: user.telegramId || '',
        notifyEmail: user.notifyEmail ?? true,
        notifyTelegram: user.notifyTelegram ?? true,
      });
    }
  }, [user]);

  const fetchEmailRecipients = useCallback(async () => {
    setEmailsLoading(true);
    try {
      const response = await imageService.getNotificationEmails();
      setEmailRecipients(response.data || []);
    } catch (error) {
      console.error('Failed to load email recipients', error);
    } finally {
      setEmailsLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchEmailRecipients();
  }, [fetchEmailRecipients]);

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setMessage({ type: '', text: '' });

    try {
      const updated = await imageService.updateConfig(formData);
      await refreshUser();
      setMessage({ type: 'success', text: 'Settings updated successfully!' });
      if (updated?.data) {
        setFormData({
          telegramId: updated.data.telegramId || '',
          notifyEmail: updated.data.notifyEmail ?? true,
          notifyTelegram: updated.data.notifyTelegram ?? true,
        });
      }
    } catch (error) {
      setMessage({ 
        type: 'error', 
        text: error.response?.data?.message || 'Failed to update settings' 
      });
    } finally {
      setLoading(false);
    }
  };

  const resetEmailForm = () => {
    setEmailForm({ address: '', label: '', active: true });
    setEditingEmailId(null);
  };

  const handleEmailSubmit = async () => {
    if (!emailForm.address.trim()) {
      setMessage({ type: 'error', text: 'Please enter a valid email address' });
      return;
    }

    setEmailsLoading(true);
    setMessage({ type: '', text: '' });

    try {
      const payload = {
        address: emailForm.address.trim(),
        label: emailForm.label.trim(),
        active: emailForm.active,
      };

      const response = editingEmailId
        ? await imageService.updateNotificationEmail(editingEmailId, payload)
        : await imageService.addNotificationEmail(payload);

      setEmailRecipients(response.data || []);
      resetEmailForm();
      setMessage({
        type: 'success',
        text: editingEmailId ? 'Recipient updated successfully!' : 'Recipient added successfully!',
      });
    } catch (error) {
      setMessage({
        type: 'error',
        text: error.response?.data?.message || 'Failed to save recipient',
      });
    } finally {
      setEmailsLoading(false);
    }
  };

  const handleEditRecipient = (recipient) => {
    setEditingEmailId(recipient.id);
    setEmailForm({
      address: recipient.address,
      label: recipient.label || '',
      active: recipient.active,
    });
  };

  const handleDeleteRecipient = async (id) => {
    if (!window.confirm('Remove this email from notifications?')) {
      return;
    }
    setEmailsLoading(true);
    try {
      const response = await imageService.deleteNotificationEmail(id);
      setEmailRecipients(response.data || []);
      setMessage({ type: 'success', text: 'Recipient removed' });
      if (editingEmailId === id) {
        resetEmailForm();
      }
    } catch (error) {
      setMessage({
        type: 'error',
        text: error.response?.data?.message || 'Failed to delete recipient',
      });
    } finally {
      setEmailsLoading(false);
    }
  };

  const handleToggleRecipient = async (recipient) => {
    setEmailsLoading(true);
    try {
      const response = await imageService.updateNotificationEmail(recipient.id, {
        active: !recipient.active,
      });
      setEmailRecipients(response.data || []);
    } catch (error) {
      setMessage({
        type: 'error',
        text: error.response?.data?.message || 'Failed to update recipient',
      });
    } finally {
      setEmailsLoading(false);
    }
  };

  const renderToggle = (checked, onChange, label, description) => (
    <div className="flex items-center justify-between mt-4 py-3 px-4 border border-gray-200 dark:border-gray-700 rounded-lg">
      <div>
        <p className="text-sm font-medium text-gray-900 dark:text-gray-100">{label}</p>
        {description && (
          <p className="text-xs text-gray-500 dark:text-gray-400 mt-1">{description}</p>
        )}
      </div>
      <button
        type="button"
        onClick={() => onChange(!checked)}
        className={`relative inline-flex h-6 w-11 flex-shrink-0 cursor-pointer rounded-full border-2 border-transparent transition-colors duration-200 focus:outline-none focus:ring-2 focus:ring-primary-500 focus:ring-offset-2 ${
          checked ? 'bg-primary-600' : 'bg-gray-300 dark:bg-gray-600'
        }`}
        role="switch"
        aria-checked={checked}
      >
        <span
          aria-hidden="true"
          className={`pointer-events-none inline-block h-5 w-5 transform rounded-full bg-white shadow ring-0 transition duration-200 ${
            checked ? 'translate-x-5' : 'translate-x-0'
          }`}
        />
      </button>
    </div>
  );

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
                <p className="text-sm text-gray-500 dark:text-gray-400 mb-3">
                  Manage who receives email alerts using the list below. Each recipient will get the same notifications.
                </p>
                {renderToggle(
                  formData.notifyEmail,
                  (value) => setFormData({ ...formData, notifyEmail: value }),
                  'Email alerts',
                  formData.notifyEmail ? 'Enabled' : 'Disabled'
                )}
              </div>
              <div className="mt-8">
                <div className="flex flex-col md:flex-row md:items-center md:justify-between">
                  <div>
                    <h3 className="text-lg font-semibold text-gray-900 dark:text-white flex items-center">
                      <Plus className="w-4 h-4 mr-2" />
                      Additional Email Recipients
                    </h3>
                    <p className="text-sm text-gray-500 dark:text-gray-400 mt-1">
                      Add extra addresses that should receive the same alerts.
                    </p>
                  </div>
                  <span className="text-sm text-gray-500 dark:text-gray-400 mt-2 md:mt-0">
                    {emailRecipients.length} saved
                  </span>
                </div>

                <div className="mt-4 grid gap-4 md:grid-cols-12">
                  <div className="md:col-span-5">
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                      Email Address
                    </label>
                    <input
                      type="email"
                      value={emailForm.address}
                      onChange={(e) => setEmailForm({ ...emailForm, address: e.target.value })}
                      className="input"
                      placeholder="friend@example.com"
                    />
                  </div>
                  <div className="md:col-span-4">
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
                      Label (optional)
                    </label>
                    <input
                      type="text"
                      value={emailForm.label}
                      onChange={(e) => setEmailForm({ ...emailForm, label: e.target.value })}
                      className="input"
                      placeholder="Front gate"
                    />
                  </div>
                  <div className="md:col-span-3 flex items-end space-x-2">
                    {renderToggle(
                      emailForm.active,
                      (value) => setEmailForm({ ...emailForm, active: value }),
                      'Active',
                      emailForm.active ? 'Enabled' : 'Paused'
                    )}
                  </div>
                  <div className="md:col-span-12 flex items-center space-x-3">
                    <button
                      type="button"
                      onClick={handleEmailSubmit}
                      disabled={emailsLoading}
                      className="btn-primary flex items-center space-x-2 disabled:opacity-50"
                    >
                      <Save className="w-4 h-4" />
                      <span>{editingEmailId ? 'Save recipient' : 'Add recipient'}</span>
                    </button>
                    {editingEmailId && (
                      <button
                        type="button"
                        onClick={resetEmailForm}
                        className="px-4 py-2 text-sm border border-gray-300 rounded-lg text-gray-700 hover:bg-gray-100 dark:text-gray-200 dark:border-gray-600 dark:hover:bg-gray-800 transition"
                      >
                        Cancel
                      </button>
                    )}
                  </div>
                </div>

                <div className="mt-6 space-y-3">
                  {emailsLoading ? (
                    <p className="text-sm text-gray-500 dark:text-gray-400">Loading recipients...</p>
                  ) : emailRecipients.length === 0 ? (
                    <p className="text-sm text-gray-500 dark:text-gray-400">
                      No additional recipients yet.
                    </p>
                  ) : (
                    emailRecipients.map((recipient) => (
                      <div
                        key={recipient.id}
                        className="flex flex-col md:flex-row md:items-center justify-between border border-gray-200 dark:border-gray-700 rounded-lg p-4"
                      >
                        <div>
                          <p className="font-medium text-gray-900 dark:text-white">{recipient.address}</p>
                          {recipient.label && (
                            <p className="text-sm text-gray-500 dark:text-gray-400">
                              Label: {recipient.label}
                            </p>
                          )}
                          <p className="text-xs text-gray-400 mt-1">
                            Added {recipient.createdAt ? new Date(recipient.createdAt).toLocaleString() : ''}
                          </p>
                        </div>
                        <div className="flex items-center space-x-2 mt-3 md:mt-0">
                          <span
                            className={`px-3 py-1 text-xs font-semibold rounded-full ${
                              recipient.active
                                ? 'bg-green-100 text-green-800 dark:bg-green-900 dark:text-green-200'
                                : 'bg-gray-200 text-gray-700 dark:bg-gray-700 dark:text-gray-200'
                            }`}
                          >
                            {recipient.active ? 'Active' : 'Paused'}
                          </span>
                          <button
                            type="button"
                            onClick={() => handleToggleRecipient(recipient)}
                            className="px-3 py-1 text-sm border border-gray-300 rounded-lg text-gray-700 hover:bg-gray-100 dark:text-gray-200 dark:border-gray-600 dark:hover:bg-gray-800 transition"
                          >
                            {recipient.active ? 'Pause' : 'Enable'}
                          </button>
                          <button
                            type="button"
                            onClick={() => handleEditRecipient(recipient)}
                            className="px-3 py-1 text-sm border border-gray-300 rounded-lg text-gray-700 hover:bg-gray-100 dark:text-gray-200 dark:border-gray-600 dark:hover:bg-gray-800 transition flex items-center space-x-1"
                          >
                            <Edit2 className="w-4 h-4" />
                            <span>Edit</span>
                          </button>
                          <button
                            type="button"
                            onClick={() => handleDeleteRecipient(recipient.id)}
                            className="text-red-600 hover:text-red-700 flex items-center space-x-1"
                          >
                            <Trash2 className="w-4 h-4" />
                            <span>Delete</span>
                          </button>
                        </div>
                      </div>
                    ))
                  )}
                </div>
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
                {renderToggle(
                  formData.notifyTelegram,
                  (value) => setFormData({ ...formData, notifyTelegram: value }),
                  'Telegram alerts',
                  formData.notifyTelegram ? 'Enabled' : 'Disabled'
                )}
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

