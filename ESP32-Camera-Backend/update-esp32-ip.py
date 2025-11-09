#!/usr/bin/env python3
"""
Auto-update ESP32 config.h with current PC WiFi IP
Run this script before uploading to ESP32
"""

import re
import subprocess

# Get WiFi IP address
def get_wifi_ip():
    try:
        result = subprocess.run(['ipconfig'], capture_output=True, text=True, shell=True)
        output = result.stdout
        
        # Find WiFi adapter section
        wifi_section = False
        for line in output.split('\n'):
            if 'Wireless LAN adapter Wi-Fi' in line:
                wifi_section = True
            elif wifi_section and 'IPv4 Address' in line:
                # Extract IP
                match = re.search(r'(\d+\.\d+\.\d+\.\d+)', line)
                if match:
                    return match.group(1)
        
        return None
    except Exception as e:
        print(f"Error getting WiFi IP: {e}")
        return None

# Update config.h
def update_config_h(new_ip):
    config_path = r"e:\A_NAM_4_KI_1\IOT\CuoiKy\ESP32-Camera-Backend\examples\modular\main\config.h"
    
    try:
        # Read file
        with open(config_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Update SERVER_IP
        content = re.sub(
            r'#define SERVER_IP "[\d\.]+"',
            f'#define SERVER_IP "{new_ip}"',
            content
        )
        
        # Update SERVER_BASE_URL
        content = re.sub(
            r'#define SERVER_BASE_URL "http://[\d\.:]+/api"',
            f'#define SERVER_BASE_URL "http://{new_ip}:3000/api"',
            content
        )
        
        # Update MQTT_BROKER
        content = re.sub(
            r'#define MQTT_BROKER "[\d\.]+"',
            f'#define MQTT_BROKER "{new_ip}"',
            content
        )
        
        # Write back
        with open(config_path, 'w', encoding='utf-8') as f:
            f.write(content)
        
        print(f"✅ Updated config.h with IP: {new_ip}")
        print(f"   SERVER_IP = {new_ip}")
        print(f"   SERVER_BASE_URL = http://{new_ip}:3000/api")
        print(f"   MQTT_BROKER = {new_ip}")
        return True
        
    except Exception as e:
        print(f"❌ Error updating config.h: {e}")
        return False

if __name__ == "__main__":
    print("🔍 Getting WiFi IP address...")
    wifi_ip = get_wifi_ip()
    
    if wifi_ip:
        print(f"📡 Found WiFi IP: {wifi_ip}")
        if update_config_h(wifi_ip):
            print("\n✅ Done! Now upload code to ESP32.")
        else:
            print("\n❌ Failed to update config.h")
    else:
        print("❌ Could not find WiFi IP address")
        print("   Make sure WiFi is connected!")
