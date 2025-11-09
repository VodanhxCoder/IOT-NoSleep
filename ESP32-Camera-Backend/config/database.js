const mongoose = require('mongoose');

const connectDB = async () => {
  try {
    const conn = await mongoose.connect(process.env.MONGO_URI, {
      serverSelectionTimeoutMS: 5000, // Timeout after 5s instead of waiting forever
    });

    console.log(`✅ MongoDB Connected: ${conn.connection.host}`);
    console.log(`   Database: ${conn.connection.name}`);
  } catch (error) {
    console.error(`❌ MongoDB Connection Error: ${error.message}`);
    console.error(`   Server will continue to run, but database operations will fail.`);
    console.error(`   Please ensure MongoDB is running on: ${process.env.MONGO_URI}`);
    // Don't exit - let server run without DB for testing
    // process.exit(1);
  }
};

module.exports = connectDB;
