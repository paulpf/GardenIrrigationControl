#!/usr/bin/env python3
"""Update mqttmanager.cpp to add Last Will Testament support"""

import os

file_path = r"d:\DEV\GITHUB\GardenIrrigationControl\src\mqttmanager.cpp"

# Read the original file
with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Find and replace the reconnect method
old_connect_line = '  if (_pubSubClient.connect(_clientName, _mqttUser, _mqttPassword))'
new_connect_lines = '''  // Phase 5.2: Last Will Testament (LWT) support for system stability
  // If device disconnects unexpectedly, broker will automatically publish "offline" to status topic
  if (_pubSubClient.connect(_clientName, _mqttUser, _mqttPassword,
                             getLwtTopic(), 1, true, getLwtOfflinePayload()))'''

content = content.replace(old_connect_line, new_connect_lines)

# Add publishOnlineStatus call after onConnectSuccess
old_success_block = '''    Trace::log(TraceLevel::INFO, "MqttManager::reconnect | MQTT connected");
    _sessionManager.onConnectSuccess();

    // Publish initial state of all irrigation zones
    initPublish();'''

new_success_block = '''    Trace::log(TraceLevel::INFO, "MqttManager::reconnect | MQTT connected");
    _sessionManager.onConnectSuccess();

    // Publish "online" status immediately after successful connection
    publishOnlineStatus();

    // Publish initial state of all irrigation zones
    initPublish();'''

content = content.replace(old_success_block, new_success_block)

# Add the LWT helper methods at the end of file (before final brace if it exists)
lwt_implementation = '''
// Phase 5.2: Last Will Testament (LWT) Support

const char *MqttManager::getLwtTopic() const
{
  // Static buffer to hold the LWT topic string
  // Format: "device_name/system/status"
  static char lwtTopic[96];
  
  // Build the topic in the static buffer
  snprintf(lwtTopic, sizeof(lwtTopic), "%s/system/status", _clientName);
  
  return lwtTopic;
}

const char *MqttManager::getLwtOnlinePayload() const
{
  return "online";
}

const char *MqttManager::getLwtOfflinePayload() const
{
  return "offline";
}

void MqttManager::publishOnlineStatus()
{
  // Immediately publish online status after successful MQTT connection
  // This serves as a health check for Home Assistant and other MQTT clients
  // If the device disconnects unexpectedly, the broker will publish "offline" 
  // based on the LWT configuration in the connect() method
  if (_sessionManager.isConnected())
  {
    Trace::log(TraceLevel::INFO, "Publishing online status to MQTT broker");
    publish(getLwtTopic(), getLwtOnlinePayload());
  }
}'''

# Append before the last closing brace
if content.endswith('}'):
    content = content[:-1] + lwt_implementation + '\n}\n'
else:
    content = content + lwt_implementation

# Write back
with open(file_path, 'w', encoding='utf-8') as f:
    f.write(content)

print("✓ Successfully updated mqttmanager.cpp with LWT support")
print("  - Updated reconnect() to use Last Will Testament")
print("  - Added publishOnlineStatus() call")
print("  - Added LWT helper methods (getLwtTopic, getLwtOnlinePayload, getLwtOfflinePayload)")
