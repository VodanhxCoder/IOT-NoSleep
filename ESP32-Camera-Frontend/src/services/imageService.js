import api from './api';
import { buildImageUrl } from '../config/env';

const resolveImageUrl = (rawUrl, path) => {
  if (rawUrl && /^https?:\/\//i.test(rawUrl)) {
    return rawUrl;
  }
  return buildImageUrl(path);
};

const withImageUrl = (image) => {
  if (!image) return image;
  const path = image.path || '';
  return {
    ...image,
    url: resolveImageUrl(image.url, path),
  };
};

const imageService = {
  uploadImage: async (file) => {
    const formData = new FormData();
    formData.append('image', file);
    const response = await api.post('/upload-image', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    if (response.data?.data?.image) {
      response.data.data.image = withImageUrl(response.data.data.image);
    }
    return response.data;
  },

  uploadSnapshot: async (blob) => {
    const formData = new FormData();
    formData.append('image', blob, 'snapshot.jpg');
    const response = await api.post('/snapshot', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    if (response.data?.data?.image) {
      response.data.data.image = withImageUrl(response.data.data.image);
    }
    return response.data;
  },

  getImages: async (page = 1) => {
    const response = await api.get(`/images?page=${page}`);
    if (response.data?.data?.images) {
      response.data.data.images = response.data.data.images.map(withImageUrl);
    }
    return response.data;
  },

  getImageById: async (id) => {
    const response = await api.get(`/images/${id}`);
    if (response.data?.data) {
      response.data.data = withImageUrl(response.data.data);
    }
    return response.data;
  },

  deleteImage: async (id) => {
    const response = await api.delete(`/images/${id}`);
    return response.data;
  },

  updateConfig: async (config) => {
    const response = await api.put('/config', config);
    return response.data;
  },

  getNotificationEmails: async () => {
    const response = await api.get('/config/emails');
    return response.data;
  },

  addNotificationEmail: async (payload) => {
    const response = await api.post('/config/emails', payload);
    return response.data;
  },

  updateNotificationEmail: async (id, payload) => {
    const response = await api.put(`/config/emails/${id}`, payload);
    return response.data;
  },

  deleteNotificationEmail: async (id) => {
    const response = await api.delete(`/config/emails/${id}`);
    return response.data;
  },

  checkNewImages: async (lastCheckTime) => {
    const response = await api.get(`/images/check-new?lastCheckTime=${lastCheckTime}`);
    if (response.data?.data?.latestImage) {
      response.data.data.latestImage = withImageUrl(response.data.data.latestImage);
    }
    return response.data;
  },
};

export default imageService;
