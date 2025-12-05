const express = require('express');
const multer = require('multer');
const path = require('path');
const fs = require('fs');
const { protect } = require('../middlewares/auth');
const {
  uploadImage,
  getImages,
  getImageById,
  deleteImage,
  updateConfig,
  checkNewImages,
  saveSnapshot
} = require('../controllers/imageController');

const router = express.Router();

// Create uploads directory if it doesn't exist
const uploadDir = path.join(__dirname, '../uploads');
if (!fs.existsSync(uploadDir)) {
  fs.mkdirSync(uploadDir, { recursive: true });
}

// Configure multer for file upload
const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, uploadDir);
  },
  filename: (req, file, cb) => {
    const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1E9);
    cb(null, 'capture-' + uniqueSuffix + path.extname(file.originalname));
  }
});

const fileFilter = (req, file, cb) => {
  // Accept only JPEG images
  if (file.mimetype === 'image/jpeg' || file.mimetype === 'image/jpg') {
    cb(null, true);
  } else {
    cb(new Error('Only JPEG images are allowed'), false);
  }
};

const upload = multer({
  storage,
  fileFilter,
  limits: {
    fileSize: 5 * 1024 * 1024 // 5MB max file size
  }
});

// Routes
router.post('/upload-image', protect, upload.single('image'), uploadImage);
router.post('/snapshot', protect, upload.single('image'), saveSnapshot);
router.get('/images/check-new', protect, checkNewImages);
router.get('/images', protect, getImages);
router.get('/images/:id', protect, getImageById);
router.delete('/images/:id', protect, deleteImage);
router.put('/config', protect, updateConfig);

module.exports = router;
