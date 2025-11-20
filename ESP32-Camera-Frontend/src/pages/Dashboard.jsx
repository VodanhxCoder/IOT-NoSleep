import React, { useState, useEffect } from 'react';
import { Camera, Image as ImageIcon, Activity, AlertCircle, RefreshCw } from 'lucide-react';
import imageService from '../services/imageService';
import { useNotification } from '../context/NotificationContext';
import { format, isSameDay } from 'date-fns';
import { buildImageUrl } from '../config/env';

const Dashboard = () => {
  const [stats, setStats] = useState({ total: 0, today: 0, detected: 0 });
  const [recentImages, setRecentImages] = useState([]);
  const [loading, setLoading] = useState(true);
  const { latestImage } = useNotification();

  useEffect(() => {
    fetchData();
  }, []);

  useEffect(() => {
    if (!latestImage) return;
    const normalized = {
      ...latestImage,
      _id: latestImage._id || latestImage.id,
      url: latestImage.url || buildImageUrl(latestImage.path || ''),
    };

    setRecentImages((prev) => {
      const exists = prev.some((img) => (img._id || img.id) === normalized._id);
      if (exists) {
        return prev;
      }
      const updated = [normalized, ...prev];
      return updated.slice(0, 6);
    });

    setStats((prev) => {
      const base = prev || { total: 0, today: 0, detected: 0 };
      const timestamp = normalized.timestamp ? new Date(normalized.timestamp) : new Date();
      const todayIncrement = isSameDay(timestamp, new Date()) ? 1 : 0;
      const detectedIncrement = normalized.detectedObject === 'person' ? 1 : 0;
      return {
        total: (base.total || 0) + 1,
        today: (base.today || 0) + todayIncrement,
        detected: (base.detected || 0) + detectedIncrement,
      };
    });
  }, [latestImage]);

  const fetchData = async () => {
    try {
      const response = await imageService.getImages(1);
      if (response.success && response.data) {
        const images = response.data.images || [];
        setRecentImages(images.slice(0, 6));
        setStats({
          total: response.data.pagination?.totalImages || 0,
          today: images.filter(img => 
            new Date(img.timestamp).toDateString() === new Date().toDateString()
          ).length,
          detected: images.filter(img => img.detectedObject === 'person').length,
        });
      }
    } catch (error) {
      console.error('Error fetching data:', error);
    } finally {
      setLoading(false);
    }
  };

  const StatCard = ({ icon: Icon, label, value, color }) => (
    <div className="card">
      <div className="flex items-center justify-between">
        <div>
          <p className="text-sm text-gray-600 dark:text-gray-400">{label}</p>
          <p className="text-3xl font-bold text-gray-900 dark:text-white mt-2">{value}</p>
        </div>
        <div className={`p-4 rounded-full ${color}`}>
          <Icon className="w-8 h-8 text-white" />
        </div>
      </div>
    </div>
  );

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8 flex justify-between items-center">
          <div>
            <h1 className="text-3xl font-bold text-gray-900 dark:text-white">Dashboard</h1>
            <p className="text-gray-600 dark:text-gray-400 mt-2">Welcome to ESP32 Security Camera System</p>
          </div>
          <button
            onClick={fetchData}
            disabled={loading}
            className="btn btn-primary flex items-center space-x-2"
          >
            <RefreshCw className={`w-5 h-5 ${loading ? 'animate-spin' : ''}`} />
            <span>Refresh</span>
          </button>
        </div>

        {/* Stats Grid */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-8">
          <StatCard
            icon={ImageIcon}
            label="Total Images"
            value={stats.total}
            color="bg-primary-600"
          />
          <StatCard
            icon={Activity}
            label="Today's Captures"
            value={stats.today}
            color="bg-green-600"
          />
          <StatCard
            icon={AlertCircle}
            label="Person Detected"
            value={stats.detected}
            color="bg-orange-600"
          />
        </div>

        {/* Recent Images */}
        <div className="card">
          <div className="flex items-center justify-between mb-6">
            <h2 className="text-2xl font-bold text-gray-900 dark:text-white">Recent Captures</h2>
            <a href="/gallery" className="text-primary-600 dark:text-primary-400 hover:underline">
              View All
            </a>
          </div>

          {loading ? (
            <div className="text-center py-12">
              <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-primary-600 mx-auto"></div>
              <p className="text-gray-600 dark:text-gray-400 mt-4">Loading...</p>
            </div>
          ) : recentImages.length === 0 ? (
            <div className="text-center py-12">
              <Camera className="w-16 h-16 text-gray-400 mx-auto mb-4" />
              <p className="text-gray-600 dark:text-gray-400">No images captured yet</p>
            </div>
          ) : (
            <div className="grid grid-cols-2 md:grid-cols-3 gap-4">
              {recentImages.map((image) => {
                const imageSrc = image?.url || buildImageUrl(image?.path || '');
                return (
                <div
                  key={image._id}
                  className="relative group rounded-lg overflow-hidden shadow-md hover:shadow-xl transition-shadow"
                >
                  <img
                    src={imageSrc}
                    alt={image.filename}
                    className="w-full h-48 object-cover"
                    onError={(e) => {
                      console.error('Image load error:', image.path || imageSrc);
                      e.target.src = 'data:image/svg+xml,%3Csvg xmlns="http://www.w3.org/2000/svg" width="200" height="200"%3E%3Crect fill="%23ddd" width="200" height="200"/%3E%3Ctext fill="%23999" x="50%25" y="50%25" text-anchor="middle" dy=".3em"%3ENo Image%3C/text%3E%3C/svg%3E';
                    }}
                  />
                  <div className="absolute inset-0 bg-black bg-opacity-0 group-hover:bg-opacity-60 transition-opacity flex items-end">
                    <div className="p-4 text-white opacity-0 group-hover:opacity-100 transition-opacity">
                      <p className="text-sm font-medium">{image.detectedObject || 'Unknown'}</p>
                      <p className="text-xs">{format(new Date(image.timestamp), 'PPpp')}</p>
                    </div>
                  </div>
                </div>
                );
              })}
            </div>
          )}
        </div>
      </div>
    </div>
  );
};

export default Dashboard;
