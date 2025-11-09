import api from './api';

const imageService = {
  uploadImage: async (file) => {
    const formData = new FormData();
    formData.append('image', file);
    const response = await api.post('/upload-image', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    return response.data;
  },

  getImages: async (page = 1) => {
    const response = await api.get(`/images?page=${page}`);
    return response.data;
  },

  getImageById: async (id) => {
    const response = await api.get(`/images/${id}`);
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

  checkNewImages: async (lastCheckTime) => {
    const response = await api.get(`/images/check-new?lastCheckTime=${lastCheckTime}`);
    return response.data;
  },
};

export default imageService;
