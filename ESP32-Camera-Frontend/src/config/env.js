const trimTrailingSlashes = (url) => (url ? url.replace(/\/+$/, '') : '');

const deriveBackendBase = (apiUrl) => {
  if (!apiUrl) {
    return 'http://localhost:3000';
  }

  try {
    const parsed = new URL(apiUrl);
    return `${parsed.protocol}//${parsed.host}`;
  } catch {
    return apiUrl.replace(/\/api\/?$/, '');
  }
};

const API_URL = trimTrailingSlashes(
  import.meta.env.VITE_API_URL || 'http://localhost:3000/api'
);

const BACKEND_BASE_URL = trimTrailingSlashes(
  import.meta.env.VITE_BACKEND_BASE_URL || deriveBackendBase(API_URL)
);

const UPLOADS_BASE_URL = trimTrailingSlashes(
  import.meta.env.VITE_UPLOADS_BASE_URL || BACKEND_BASE_URL
);

const LIVE_STREAM_URL =
  import.meta.env.VITE_LIVE_STREAM_URL || `${BACKEND_BASE_URL}/api/live`;

const buildImageUrl = (relativePath = '') => {
  if (!relativePath) return '';
  const normalizedPath = relativePath.startsWith('/')
    ? relativePath
    : `/${relativePath}`;
  return `${UPLOADS_BASE_URL}${normalizedPath}`;
};

export {
  API_URL,
  BACKEND_BASE_URL,
  UPLOADS_BASE_URL,
  LIVE_STREAM_URL,
  buildImageUrl,
};
