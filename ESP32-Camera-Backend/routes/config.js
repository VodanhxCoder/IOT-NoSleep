const express = require('express');
const router = express.Router();
const { protect } = require('../middlewares/auth');
const {
  updateConfig,
  getEmails,
  addEmail,
  updateEmail,
  deleteEmail
} = require('../controllers/configController');

router.use(protect); // All routes are protected

router.put('/', updateConfig);
router.get('/emails', getEmails);
router.post('/emails', addEmail);
router.put('/emails/:id', updateEmail);
router.delete('/emails/:id', deleteEmail);

module.exports = router;
