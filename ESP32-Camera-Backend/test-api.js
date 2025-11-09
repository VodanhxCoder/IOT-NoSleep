#!/usr/bin/env node

/**
 * Test Script for ESP32 Security Camera Backend
 * Run with: node test-api.js
 */

const axios = require('axios');
const FormData = require('form-data');
const fs = require('fs');
const path = require('path');

const BASE_URL = 'http://localhost:3000';
let authToken = '';
let userId = '';

// Colors for console output
const colors = {
  reset: '\x1b[0m',
  green: '\x1b[32m',
  red: '\x1b[31m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m'
};

const log = {
  success: (msg) => console.log(`${colors.green}✓${colors.reset} ${msg}`),
  error: (msg) => console.log(`${colors.red}✗${colors.reset} ${msg}`),
  info: (msg) => console.log(`${colors.blue}ℹ${colors.reset} ${msg}`),
  warn: (msg) => console.log(`${colors.yellow}⚠${colors.reset} ${msg}`)
};

// Test data
const testUser = {
  username: 'testuser_' + Date.now(),
  password: 'password123',
  email: `test${Date.now()}@example.com`,
  telegramId: '123456789'
};

// Helper to create test image
function createTestImage() {
  const testImagePath = path.join(__dirname, 'test-image.jpg');
  
  if (!fs.existsSync(testImagePath)) {
    // Create a minimal valid JPEG file for testing
    const jpegHeader = Buffer.from([
      0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,
      0x49, 0x46, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
      0x00, 0x01, 0x00, 0x00, 0xFF, 0xD9
    ]);
    fs.writeFileSync(testImagePath, jpegHeader);
    log.warn('Created minimal test image');
  }
  
  return testImagePath;
}

// Test functions
async function testHealthCheck() {
  try {
    const response = await axios.get(`${BASE_URL}/health`);
    if (response.data.success) {
      log.success('Health check passed');
      return true;
    }
  } catch (error) {
    log.error(`Health check failed: ${error.message}`);
    return false;
  }
}

async function testRegister() {
  try {
    const response = await axios.post(`${BASE_URL}/api/auth/register`, testUser);
    
    if (response.data.success && response.data.data.token) {
      authToken = response.data.data.token;
      userId = response.data.data.user.id;
      log.success(`User registered: ${testUser.username}`);
      return true;
    }
  } catch (error) {
    log.error(`Registration failed: ${error.response?.data?.message || error.message}`);
    return false;
  }
}

async function testLogin() {
  try {
    const response = await axios.post(`${BASE_URL}/api/auth/login`, {
      username: testUser.username,
      password: testUser.password
    });
    
    if (response.data.success && response.data.data.token) {
      authToken = response.data.data.token;
      log.success(`Login successful for: ${testUser.username}`);
      return true;
    }
  } catch (error) {
    log.error(`Login failed: ${error.response?.data?.message || error.message}`);
    return false;
  }
}

async function testGetMe() {
  try {
    const response = await axios.get(`${BASE_URL}/api/auth/me`, {
      headers: { Authorization: `Bearer ${authToken}` }
    });
    
    if (response.data.success) {
      log.success(`Get current user successful: ${response.data.data.username}`);
      return true;
    }
  } catch (error) {
    log.error(`Get me failed: ${error.response?.data?.message || error.message}`);
    return false;
  }
}

async function testUploadImage() {
  try {
    const testImagePath = createTestImage();
    const formData = new FormData();
    formData.append('image', fs.createReadStream(testImagePath));
    
    const response = await axios.post(`${BASE_URL}/api/upload-image`, formData, {
      headers: {
        ...formData.getHeaders(),
        Authorization: `Bearer ${authToken}`
      }
    });
    
    log.success(`Image upload: ${response.data.message}`);
    return true;
  } catch (error) {
    log.error(`Upload failed: ${error.response?.data?.message || error.message}`);
    return false;
  }
}

async function testGetImages() {
  try {
    const response = await axios.get(`${BASE_URL}/api/images?page=1&limit=10`, {
      headers: { Authorization: `Bearer ${authToken}` }
    });
    
    if (response.data.success) {
      const count = response.data.data.images.length;
      log.success(`Retrieved ${count} images`);
      return true;
    }
  } catch (error) {
    log.error(`Get images failed: ${error.response?.data?.message || error.message}`);
    return false;
  }
}

async function testUpdateConfig() {
  try {
    const response = await axios.put(`${BASE_URL}/api/config`, {
      email: 'newemail@example.com',
      telegramId: '987654321'
    }, {
      headers: { Authorization: `Bearer ${authToken}` }
    });
    
    if (response.data.success) {
      log.success('Configuration updated successfully');
      return true;
    }
  } catch (error) {
    log.error(`Update config failed: ${error.response?.data?.message || error.message}`);
    return false;
  }
}

async function testUnauthorizedAccess() {
  try {
    await axios.get(`${BASE_URL}/api/images`);
    log.error('Unauthorized access should have been blocked');
    return false;
  } catch (error) {
    if (error.response?.status === 401) {
      log.success('Unauthorized access properly blocked');
      return true;
    }
    log.error(`Unexpected error: ${error.message}`);
    return false;
  }
}

// Main test runner
async function runTests() {
  console.log('\n' + '='.repeat(60));
  console.log('ESP32 Security Camera Backend - API Test Suite');
  console.log('='.repeat(60) + '\n');
  
  const tests = [
    { name: 'Health Check', fn: testHealthCheck },
    { name: 'User Registration', fn: testRegister },
    { name: 'User Login', fn: testLogin },
    { name: 'Get Current User', fn: testGetMe },
    { name: 'Upload Image', fn: testUploadImage },
    { name: 'Get Images', fn: testGetImages },
    { name: 'Update Config', fn: testUpdateConfig },
    { name: 'Unauthorized Access', fn: testUnauthorizedAccess }
  ];
  
  let passed = 0;
  let failed = 0;
  
  for (const test of tests) {
    log.info(`Running: ${test.name}`);
    const result = await test.fn();
    if (result) {
      passed++;
    } else {
      failed++;
    }
    console.log('');
  }
  
  console.log('='.repeat(60));
  console.log(`Tests completed: ${passed} passed, ${failed} failed`);
  console.log('='.repeat(60) + '\n');
  
  // Cleanup test image
  const testImagePath = path.join(__dirname, 'test-image.jpg');
  if (fs.existsSync(testImagePath)) {
    fs.unlinkSync(testImagePath);
  }
  
  process.exit(failed > 0 ? 1 : 0);
}

// Run tests
runTests().catch((error) => {
  log.error(`Test suite failed: ${error.message}`);
  process.exit(1);
});
