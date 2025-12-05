import React, { useState, useEffect, useRef } from 'react';
import { Video, VideoOff, RefreshCw, Info, Camera, CheckCircle, AlertCircle } from 'lucide-react';
import { LIVE_STREAM_URL } from '../config/env';
import { getSocket } from '../services/socketService';
import imageService from '../services/imageService';

const LiveStream = () => {
  const [isConnected, setIsConnected] = useState(false);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const [debugInfo, setDebugInfo] = useState({ status: 'unknown', ip: 'unknown' });
  const [snapshotting, setSnapshotting] = useState(false);
  const [notification, setNotification] = useState(null);
  const imgRef = useRef(null);

  // WebSocket Stream State
  const wsUrlRef = useRef(null);

  const [retryCount, setRetryCount] = useState(0);
  const retryTimeoutRef = useRef(null);

  const streamUrl = LIVE_STREAM_URL;

  useEffect(() => {
    const socket = getSocket();
    if (!socket.connected) socket.connect();

    // Handle WebSocket Stream Frames
    const handleStreamFrame = (frameData) => {
      try {
        const blob = new Blob([frameData], { type: 'image/jpeg' });
        const url = URL.createObjectURL(blob);
        
        if (wsUrlRef.current) URL.revokeObjectURL(wsUrlRef.current);
        wsUrlRef.current = url;
        
        if (imgRef.current) {
          imgRef.current.src = url;
          if (!isConnected) {
             setIsConnected(true);
             setLoading(false);
             setError('');
          }
        }
      } catch (e) {
        console.error("Error processing WS frame:", e);
      }
    };

    socket.on('stream-frame', handleStreamFrame);

    // Notify backend of our viewing mode
    socket.emit('join-stream');

    // Listen for ESP32 status updates
    const handleStatus = (status) => {
      console.log('ESP32 Status:', status);
      setDebugInfo(prev => ({ ...prev, ...status }));
    };

    socket.on('esp32-status', handleStatus);

    // Initial connection
    setLoading(true); // Wait for frames
    setError('');

    return () => {
      if (retryTimeoutRef.current) clearTimeout(retryTimeoutRef.current);
      socket.off('esp32-status', handleStatus);
      socket.off('stream-frame', handleStreamFrame);
      socket.emit('leave-stream');
      if (wsUrlRef.current) URL.revokeObjectURL(wsUrlRef.current);
    };
  }, []);

  const showNotification = (message, type = 'success') => {
    setNotification({ message, type });
    setTimeout(() => setNotification(null), 3000);
  };

  const handleSnapshot = async () => {
    if (!imgRef.current || !isConnected) return;

    try {
      setSnapshotting(true);
      
      // Create a canvas to capture the frame
      const canvas = document.createElement('canvas');
      canvas.width = imgRef.current.naturalWidth;
      canvas.height = imgRef.current.naturalHeight;
      const ctx = canvas.getContext('2d');
      
      // Draw the current image frame to canvas
      ctx.drawImage(imgRef.current, 0, 0);
      
      // Convert to blob
      canvas.toBlob(async (blob) => {
        if (blob) {
          try {
            await imageService.uploadSnapshot(blob);
            showNotification('Snapshot saved successfully!', 'success');
          } catch (err) {
            console.error('Snapshot upload failed:', err);
            showNotification('Failed to save snapshot', 'error');
          } finally {
            setSnapshotting(false);
          }
        }
      }, 'image/jpeg', 0.95);
      
    } catch (err) {
      console.error('Snapshot failed:', err);
      setSnapshotting(false);
    }
  };

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900 relative">
      {/* Notification Popup */}
      {notification && (
        <div className={`fixed top-20 right-4 z-50 px-6 py-4 rounded-lg shadow-xl transform transition-all duration-300 ease-in-out flex items-center space-x-3 border ${
          notification.type === 'success' 
            ? 'bg-white dark:bg-gray-800 border-green-500 text-green-700 dark:text-green-400' 
            : 'bg-white dark:bg-gray-800 border-red-500 text-red-700 dark:text-red-400'
        }`}>
          {notification.type === 'success' ? (
            <CheckCircle className="w-6 h-6 text-green-500" />
          ) : (
            <AlertCircle className="w-6 h-6 text-red-500" />
          )}
          <div>
            <h4 className="font-bold text-sm">{notification.type === 'success' ? 'Success' : 'Error'}</h4>
            <p className="text-sm opacity-90">{notification.message}</p>
          </div>
          <button 
            onClick={() => setNotification(null)}
            className="ml-4 text-gray-400 hover:text-gray-600 dark:hover:text-gray-200"
          >
            <span className="sr-only">Close</span>
            <svg className="w-4 h-4" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M6 18L18 6M6 6l12 12" />
            </svg>
          </button>
        </div>
      )}

      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8 flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold text-gray-900 dark:text-white">Live Stream</h1>
            <p className="text-gray-600 dark:text-gray-400 mt-2">Real-time view from ESP32 camera</p>
          </div>
          
          <div className="flex items-center space-x-4">
            <div className="flex items-center space-x-2">
              <div className={`w-3 h-3 rounded-full ${isConnected ? 'bg-green-500 animate-pulse' : 'bg-red-500'}`} />
              <span className="text-sm text-gray-700 dark:text-gray-300">
                {isConnected ? 'Connected' : 'Disconnected'}
              </span>
            </div>
            
            <button
              onClick={handleSnapshot}
              disabled={!isConnected || snapshotting}
              className="btn-primary flex items-center space-x-2"
            >
              <Camera className={`w-4 h-4 ${snapshotting ? 'animate-pulse' : ''}`} />
              <span>{snapshotting ? 'Saving...' : 'Snapshot'}</span>
            </button>
          </div>
        </div>

        {/* Debug Info Panel */}
        <div className="bg-blue-50 dark:bg-blue-900/20 p-4 rounded-lg mb-4 flex items-start space-x-3">
          <Info className="w-5 h-5 text-blue-500 mt-0.5" />
          <div className="text-sm text-blue-700 dark:text-blue-300">
            <p><strong>Stream URL:</strong> {streamUrl}</p>
            <p><strong>ESP32 Status:</strong> {debugInfo.status || 'Unknown'}</p>
            <p><strong>ESP32 IP:</strong> {debugInfo.ip || 'Unknown'}</p>
            <p className="text-xs mt-1 opacity-75">
              Note: If stream fails, check if ESP32 is connected to MQTT and sending status.
            </p>
          </div>
        </div>

        <div className="card">
          <div className="relative bg-black rounded-lg overflow-hidden" style={{ paddingBottom: '56.25%' }}>
            {loading && (
              <div className="absolute inset-0 flex items-center justify-center">
                <div className="text-center">
                  <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-primary-600 mx-auto mb-4"></div>
                  <p className="text-white">Connecting to stream...</p>
                </div>
              </div>
            )}

            {error && (
              <div className="absolute inset-0 flex items-center justify-center bg-gray-900">
                <div className="text-center p-8">
                  <VideoOff className="w-16 h-16 text-red-500 mx-auto mb-4" />
                  <p className="text-white mb-2 font-medium">Stream Unavailable</p>
                  <p className="text-gray-400 text-sm mb-4">{error}</p>
                  <div className="flex flex-col gap-2">
                    <div className="flex items-center justify-center gap-2 text-yellow-500">
                       <RefreshCw className="w-4 h-4 animate-spin" />
                       <span className="text-sm">Waiting for stream...</span>
                    </div>
                  </div>
                </div>
              </div>
            )}

            <img
              ref={imgRef}
              alt="ESP32 Live Stream"
              className="absolute inset-0 w-full h-full object-contain"
              style={{ display: isConnected ? 'block' : 'none' }}
            />
          </div>

          {isConnected && (
            <div className="mt-4 p-4 bg-green-50 dark:bg-green-900 rounded-lg">
              <div className="flex items-center space-x-2">
                <Video className="w-5 h-5 text-green-600 dark:text-green-400" />
                <span className="text-sm text-green-800 dark:text-green-200">
                  Live stream is active. Viewing real-time feed from ESP32 camera.
                </span>
              </div>
            </div>
          )}
        </div>

        {/* Stream Info */}
        <div className="mt-6 grid grid-cols-1 md:grid-cols-3 gap-4">
          <div className="card">
            <h3 className="font-semibold text-gray-900 dark:text-white mb-2">Stream URL</h3>
            <p className="text-sm text-gray-600 dark:text-gray-400 break-all">{streamUrl}</p>
          </div>
          <div className="card">
            <h3 className="font-semibold text-gray-900 dark:text-white mb-2">Format</h3>
            <p className="text-sm text-gray-600 dark:text-gray-400">
                MJPEG (WebSocket)
            </p>
          </div>
          <div className="card">
            <h3 className="font-semibold text-gray-900 dark:text-white mb-2">Status</h3>
            <p className={`text-sm font-medium ${isConnected ? 'text-green-600' : 'text-red-600'}`}>
              {isConnected ? 'Active' : 'Inactive'}
            </p>
          </div>
        </div>
      </div>
    </div>
  );
};

export default LiveStream;
