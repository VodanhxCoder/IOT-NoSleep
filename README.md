

# ESP32 Camera Monitoring System

An end-to-end surveillance platform built around the ESP32-S3-EYE module. The system captures frames from the camera, pushes them through an MQTT-secured pipeline, persists important snapshots in MongoDB, and serves a responsive React dashboard for live viewing, gallery browsing, and alert management. Backend services expose REST and WebSocket APIs for device control, authentication, and notification workflows (email, Telegram).

## Quick Start

1. **Backend**
   - `cd ESP32-Camera-Backend`
   - `npm install`
   - Copy `.env.example` to `.env`, then fill in MongoDB URI, JWT secret, SMTP, and Telegram tokens.
   - Start in development: `npm run dev`
   - Optional Docker: `docker compose up` (uses `docker-compose.yml`).

2. **Frontend**
   - `cd ESP32-Camera-Frontend`
   - `npm install`
   - Create `.env` from `.env.example` and point `VITE_API_URL` to the backend.
   - Run dev server: `npm run dev`

3. **ESP32 Device**
   - Flash the firmware under `ESP32-Camera-Backend/examples/modular/main` (follow `README.md`).
   - Update the device IP via the backend script `update-esp32-ip.py` if needed.

## Tech Stack

- Node.js, Express, Socket.IO, MQTT, MongoDB, Mongoose
- React 18, Vite, Tailwind CSS, React Router
- Multer for uploads, JWT authentication, Nodemailer, Telegram Bot API

## Deployment Options
- **Manual Node processes** using `npm run dev` / `npm start` on separate hosts.
- **Reverse proxy** or tunnel (ngrok) can expose the backend, while MQTT brokers can run locally or in the cloud.


