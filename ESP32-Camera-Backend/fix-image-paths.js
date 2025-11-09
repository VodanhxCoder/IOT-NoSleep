// Script to fix image paths in MongoDB
// Run with: node fix-image-paths.js

const mongoose = require('mongoose');

const MONGO_URI = 'mongodb+srv://esp32_security:%40Khueqp16102004@cluster0.w3rf7g3.mongodb.net/esp32_security?retryWrites=true&w=majority';

const imageSchema = new mongoose.Schema({
  filename: String,
  path: String,
  size: Number,
  timestamp: Date,
  detectedObject: String,
  notificationSent: Boolean,
});

const Image = mongoose.model('Image', imageSchema);

async function fixPaths() {
  try {
    console.log('Connecting to MongoDB...');
    await mongoose.connect(MONGO_URI);
    console.log('✅ Connected!');

    // Find all images with broken paths
    const images = await Image.find({ path: /^\/\/app\/uploads\// });
    console.log(`Found ${images.length} images with broken paths`);

    if (images.length === 0) {
      console.log('No broken paths to fix!');
      return;
    }

    // Fix each image path
    let fixed = 0;
    for (const image of images) {
      const oldPath = image.path;
      // Convert "//app/uploads/file.jpg" to "/uploads/file.jpg"
      const newPath = oldPath.replace(/^\/\/app\/uploads\//, '/uploads/');
      
      await Image.updateOne(
        { _id: image._id },
        { $set: { path: newPath } }
      );
      
      console.log(`Fixed: ${oldPath} → ${newPath}`);
      fixed++;
    }

    console.log(`\n✅ Fixed ${fixed} image paths!`);
    
  } catch (error) {
    console.error('Error:', error);
  } finally {
    await mongoose.disconnect();
    console.log('Disconnected from MongoDB');
  }
}

fixPaths();
