import React, { createContext, useContext, useState, useEffect } from 'react';
import { useAuth } from './AuthContext';
import imageService from '../services/imageService';

const NotificationContext = createContext();

export const useNotification = () => {
  const context = useContext(NotificationContext);
  if (!context) {
    throw new Error('useNotification must be used within NotificationProvider');
  }
  return context;
};

export const NotificationProvider = ({ children }) => {
  const [newImagesCount, setNewImagesCount] = useState(0);
  const [lastCheckTime, setLastCheckTime] = useState(Date.now());
  const [latestImage, setLatestImage] = useState(null);
  const { isAuthenticated } = useAuth();

  // Auto-check for new images every 5 seconds
  useEffect(() => {
    if (!isAuthenticated) {
      setNewImagesCount(0);
      return;
    }

    const checkForNewImages = async () => {
      try {
        const response = await imageService.checkNewImages(lastCheckTime);
        if (response.success && response.data) {
          const { hasNewImages, newImagesCount: count, latestImage: latest } = response.data;
          
          if (hasNewImages) {
            setNewImagesCount(count);
            setLatestImage(latest);
            
            // Show browser notification
            if ('Notification' in window && Notification.permission === 'granted') {
              new Notification('New Image Captured! 📸', {
                body: `${count} new image(s) detected`,
                icon: '/camera-icon.png',
                tag: 'new-image'
              });
            }
          }
        }
      } catch (error) {
        console.error('Error checking for new images:', error);
      }
    };

    // Initial check
    checkForNewImages();

    // Set up polling interval
    const interval = setInterval(checkForNewImages, 5000); // Check every 5 seconds

    return () => clearInterval(interval);
  }, [isAuthenticated, lastCheckTime]);

  // Request notification permission on mount
  useEffect(() => {
    if ('Notification' in window && Notification.permission === 'default') {
      Notification.requestPermission();
    }
  }, []);

  const clearNotifications = () => {
    setNewImagesCount(0);
    setLastCheckTime(Date.now());
    setLatestImage(null);
  };

  const value = {
    newImagesCount,
    latestImage,
    clearNotifications
  };

  return (
    <NotificationContext.Provider value={value}>
      {children}
    </NotificationContext.Provider>
  );
};
