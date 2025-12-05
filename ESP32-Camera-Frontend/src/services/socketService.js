import { io } from 'socket.io-client';
import { BACKEND_BASE_URL } from '../config/env';

let socket = null;

const createSocket = () =>
  io(BACKEND_BASE_URL, {
    autoConnect: false,
    transports: ['websocket'],
    reconnectionAttempts: 5,
  });

export const getSocket = () => {
  if (!socket) {
    socket = createSocket();
  }
  return socket;
};

export const connectSocket = () => {
  const instance = getSocket();
  if (!instance.connected) {
    instance.connect();
  }
  return instance;
};

export const disconnectSocket = () => {
  if (socket) {
    socket.disconnect();
    socket = null;
  }
};
