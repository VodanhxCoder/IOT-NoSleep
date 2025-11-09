import React, { useState, useEffect, useRef } from 'react';
import { Video, VideoOff, AlertCircle, RefreshCw } from 'lucide-react';

const LiveStream = () => {
  const [isConnected, setIsConnected] = useState(false);
  const [error, setError] = useState('');
  const [loading, setLoading] = useState(false);
  const imgRef = useRef(null);

  const streamUrl = 'http://localhost:3000/api/live';

  useEffect(() => {
    connectStream();
    return () => disconnectStream();
  }, []);

  const connectStream = () => {
    setLoading(true);
    setError('');
    
    if (imgRef.current) {
      imgRef.current.onload = () => {
        setIsConnected(true);
        setLoading(false);
      };
      
      imgRef.current.onerror = () => {
        setError('Failed to connect to camera stream. Make sure ESP32 is online.');
        setIsConnected(false);
        setLoading(false);
      };

      imgRef.current.src = `${streamUrl}?t=${Date.now()}`;
    }
  };

  const disconnectStream = () => {
    if (imgRef.current) {
      imgRef.current.src = '';
    }
    setIsConnected(false);
  };

  const handleReconnect = () => {
    disconnectStream();
    setTimeout(connectStream, 500);
  };

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900">
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
              onClick={handleReconnect}
              disabled={loading}
              className="btn-secondary flex items-center space-x-2"
            >
              <RefreshCw className={`w-4 h-4 ${loading ? 'animate-spin' : ''}`} />
              <span>Reconnect</span>
            </button>
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
                  <button onClick={handleReconnect} className="btn-primary">
                    Try Again
                  </button>
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
            <p className="text-sm text-gray-600 dark:text-gray-400">MJPEG Stream</p>
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
