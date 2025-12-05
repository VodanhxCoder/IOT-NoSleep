import api from './api';

const userService = {
  getUsers: async () => {
    const response = await api.get('/users');
    return response.data;
  },

  getOnlineUsers: async () => {
    const response = await api.get('/users/online');
    return response.data;
  },

  createUser: async (userData) => {
    const response = await api.post('/users', userData);
    return response.data;
  },

  banUser: async (id) => {
    const response = await api.put(`/users/${id}/ban`);
    return response.data;
  },

  unbanUser: async (id) => {
    const response = await api.put(`/users/${id}/unban`);
    return response.data;
  },

  deleteUser: async (id) => {
    const response = await api.delete(`/users/${id}`);
    return response.data;
  }
};

export default userService;
