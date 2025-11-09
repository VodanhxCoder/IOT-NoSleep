import React, { useState, useEffect } from 'react';
import { Image as ImageIcon, Trash2, Download, Calendar, Search, RefreshCw } from 'lucide-react';
import imageService from '../services/imageService';
import { useNotification } from '../context/NotificationContext';
import { format } from 'date-fns';

const Gallery = () => {
  const [images, setImages] = useState([]);
  const [loading, setLoading] = useState(true);
  const [page, setPage] = useState(1);
  const [pagination, setPagination] = useState(null);
  const [searchTerm, setSearchTerm] = useState('');
  const [selectedImage, setSelectedImage] = useState(null);
  const { newImagesCount, clearNotifications } = useNotification();

  useEffect(() => {
    fetchImages();
    // Clear notifications when entering gallery
    clearNotifications();
  }, [page]);

  // Auto-refresh when new images detected
  useEffect(() => {
    if (newImagesCount > 0 && page === 1) {
      fetchImages();
    }
  }, [newImagesCount]);

  const fetchImages = async () => {
    try {
      setLoading(true);
      const response = await imageService.getImages(page);
      if (response.success && response.data) {
        setImages(response.data.images || []);
        setPagination(response.data.pagination);
      }
    } catch (error) {
      console.error('Error fetching images:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleDelete = async (id) => {
    if (!window.confirm('Are you sure you want to delete this image?')) return;

    try {
      await imageService.deleteImage(id);
      fetchImages();
    } catch (error) {
      console.error('Error deleting image:', error);
      alert('Failed to delete image');
    }
  };

  const handleDownload = async (image) => {
    try {
      const response = await fetch(`http://localhost:3000${image.path}`, {
        headers: {
          'Authorization': `Bearer ${localStorage.getItem('token')}`
        }
      });
      
      if (!response.ok) throw new Error('Download failed');
      
      const blob = await response.blob();
      const url = window.URL.createObjectURL(blob);
      const a = document.createElement('a');
      a.href = url;
      a.download = image.filename || 'image.jpg';
      document.body.appendChild(a);
      a.click();
      window.URL.revokeObjectURL(url);
      document.body.removeChild(a);
    } catch (error) {
      console.error('Error downloading image:', error);
      alert('Failed to download image');
    }
  };

  const filteredImages = (images || []).filter(img => 
    img.filename?.toLowerCase().includes(searchTerm.toLowerCase()) ||
    img.detectedObject?.toLowerCase().includes(searchTerm.toLowerCase())
  );

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-8">
        <div className="mb-8 flex justify-between items-center">
          <div>
            <h1 className="text-3xl font-bold text-gray-900 dark:text-white">Image Gallery</h1>
            <p className="text-gray-600 dark:text-gray-400 mt-2">All captured images from your security camera</p>
          </div>
          <button
            onClick={fetchImages}
            disabled={loading}
            className="btn btn-primary flex items-center space-x-2"
          >
            <RefreshCw className={`w-5 h-5 ${loading ? 'animate-spin' : ''}`} />
            <span>Refresh</span>
          </button>
        </div>

        {/* Search */}
        <div className="mb-6">
          <div className="relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 w-5 h-5 text-gray-400" />
            <input
              type="text"
              placeholder="Search images..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              className="input pl-10"
            />
          </div>
        </div>

        {loading ? (
          <div className="text-center py-12">
            <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-primary-600 mx-auto"></div>
            <p className="text-gray-600 dark:text-gray-400 mt-4">Loading images...</p>
          </div>
        ) : filteredImages.length === 0 ? (
          <div className="card text-center py-12">
            <ImageIcon className="w-16 h-16 text-gray-400 mx-auto mb-4" />
            <p className="text-gray-600 dark:text-gray-400">No images found</p>
          </div>
        ) : (
          <>
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 xl:grid-cols-4 gap-6">
              {filteredImages.map((image) => (
                <div key={image._id} className="card p-0 overflow-hidden group">
                  <div className="relative aspect-video">
                    <img
                      src={`http://localhost:3000${image.path}`}
                      alt={image.filename}
                      className="w-full h-full object-cover cursor-pointer"
                      onClick={() => setSelectedImage(image)}
                      onError={(e) => {
                        console.error('Image load error:', image.path);
                        e.target.src = 'data:image/svg+xml,%3Csvg xmlns="http://www.w3.org/2000/svg" width="400" height="300"%3E%3Crect fill="%23ddd" width="400" height="300"/%3E%3Ctext fill="%23999" x="50%25" y="50%25" text-anchor="middle" dy=".3em"%3EImage Not Found%3C/text%3E%3C/svg%3E';
                      }}
                    />
                    <div className="absolute inset-0 bg-black bg-opacity-0 group-hover:bg-opacity-50 transition-opacity flex items-center justify-center opacity-0 group-hover:opacity-100">
                      <button
                        onClick={() => setSelectedImage(image)}
                        className="btn-primary mr-2"
                      >
                        View
                      </button>
                      <button
                        onClick={() => handleDelete(image._id)}
                        className="bg-red-600 hover:bg-red-700 text-white px-4 py-2 rounded-lg"
                      >
                        <Trash2 className="w-4 h-4" />
                      </button>
                    </div>
                  </div>
                  <div className="p-4">
                    <p className="font-medium text-gray-900 dark:text-white truncate">
                      {image.filename}
                    </p>
                    <div className="mt-2 flex items-center justify-between text-sm text-gray-600 dark:text-gray-400">
                      <span className="flex items-center">
                        <Calendar className="w-4 h-4 mr-1" />
                        {format(new Date(image.timestamp), 'MMM d, yyyy')}
                      </span>
                      {image.detectedObject && (
                        <span className="bg-primary-100 dark:bg-primary-900 text-primary-700 dark:text-primary-300 px-2 py-1 rounded text-xs font-medium">
                          {image.detectedObject}
                        </span>
                      )}
                    </div>
                  </div>
                </div>
              ))}
            </div>

            {/* Pagination */}
            {pagination && pagination.pages > 1 && (
              <div className="mt-8 flex justify-center space-x-2">
                <button
                  onClick={() => setPage(page - 1)}
                  disabled={page === 1}
                  className="btn-secondary disabled:opacity-50"
                >
                  Previous
                </button>
                <span className="px-4 py-2 text-gray-700 dark:text-gray-300">
                  Page {page} of {pagination.pages}
                </span>
                <button
                  onClick={() => setPage(page + 1)}
                  disabled={page === pagination.pages}
                  className="btn-secondary disabled:opacity-50"
                >
                  Next
                </button>
              </div>
            )}
          </>
        )}

        {/* Image Modal */}
        {selectedImage && (
          <div
            className="fixed inset-0 bg-black bg-opacity-75 flex items-center justify-center z-50 p-4"
            onClick={() => setSelectedImage(null)}
          >
            <div
              className="bg-white dark:bg-gray-800 rounded-lg max-w-4xl w-full max-h-[90vh] overflow-auto"
              onClick={(e) => e.stopPropagation()}
            >
              <div className="p-6">
                <img
                  src={`http://localhost:3000${selectedImage.path}`}
                  alt={selectedImage.filename}
                  className="w-full rounded-lg mb-4"
                />
                <h3 className="text-xl font-bold text-gray-900 dark:text-white mb-2">
                  {selectedImage.filename}
                </h3>
                <div className="space-y-2 text-gray-600 dark:text-gray-400">
                  <p><strong>Timestamp:</strong> {format(new Date(selectedImage.timestamp), 'PPpp')}</p>
                  {selectedImage.detectedObject && (
                    <p><strong>Detected:</strong> {selectedImage.detectedObject}</p>
                  )}
                </div>
                <div className="mt-6 flex space-x-3">
                  <button
                    onClick={() => handleDownload(selectedImage)}
                    className="btn-primary flex items-center space-x-2"
                  >
                    <Download className="w-4 h-4" />
                    <span>Download</span>
                  </button>
                  <button
                    onClick={() => {
                      handleDelete(selectedImage._id);
                      setSelectedImage(null);
                    }}
                    className="bg-red-600 hover:bg-red-700 text-white px-4 py-2 rounded-lg flex items-center space-x-2"
                  >
                    <Trash2 className="w-4 h-4" />
                    <span>Delete</span>
                  </button>
                  <button
                    onClick={() => setSelectedImage(null)}
                    className="btn-secondary"
                  >
                    Close
                  </button>
                </div>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default Gallery;
