# Test Notification System

## Steps to Test:

1. **Open Frontend**: http://localhost:5174
2. **Login**: admin / admin123
3. **Open Browser Console** (F12)
4. **Watch for logs**:
   - 🔍 Checking for new images...
   - 📥 Check response:
   - 🆕 New images found! (if any)

## Test Upload from ESP32:
1. Trigger PIR sensor or reset ESP32
2. Wait for image upload
3. Check backend logs:
   ```bash
   docker logs esp32_backend --tail 20 -f
   ```
4. Within 5 seconds, frontend should show notification badge

## Manual Test API:
```powershell
# Login first
$response = Invoke-RestMethod -Uri "http://localhost:3000/api/auth/login" -Method Post -ContentType "application/json" -Body '{"username":"admin","password":"admin123"}'
$token = $response.data.token

# Check for new images
Invoke-RestMethod -Uri "http://localhost:3000/api/images/check-new?lastCheckTime=1731132000000" -Method Get -Headers @{"Authorization"="Bearer $token"}
```

## Expected Behavior:
- ✅ Notification badge appears in navbar (red circle with count)
- ✅ Bell icon bounces
- ✅ Dashboard auto-refreshes
- ✅ Gallery auto-refreshes if on page 1
- ✅ Browser notification pops up (if permission granted)

## Troubleshooting:

### No notification appears:
1. Check console for errors
2. Verify token is valid: `localStorage.getItem('token')`
3. Check backend is running: `docker ps`
4. Test API manually (see above)

### Download not working:
- Fixed: Now uses fetch + blob instead of direct link
- Requires authentication token
- Should work for all image types

### Images not auto-refreshing:
1. Check NotificationContext is mounted in App.jsx
2. Verify polling interval (5 seconds)
3. Check console logs for "🔍 Checking for new images..."
4. Ensure user is logged in

## Current Status:
- Backend API: ✅ Working
- Frontend Context: ✅ Mounted
- Polling: ✅ Every 5 seconds
- Download: ✅ Fixed with blob
- Auto-refresh: ⏳ Testing needed
