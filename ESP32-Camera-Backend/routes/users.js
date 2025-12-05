const express = require('express');
const router = express.Router();
const {
  getUsers,
  createUser,
  banUser,
  unbanUser,
  deleteUser,
  getOnlineUsers
} = require('../controllers/userController');
const { protect } = require('../middlewares/auth');
const { admin } = require('../middlewares/admin');

// All routes are protected and require admin privileges
router.use(protect);
router.use(admin);

router.route('/')
  .get(getUsers)
  .post(createUser);

router.get('/online', getOnlineUsers);

router.route('/:id')
  .delete(deleteUser);

router.put('/:id/ban', banUser);
router.put('/:id/unban', unbanUser);

module.exports = router;
