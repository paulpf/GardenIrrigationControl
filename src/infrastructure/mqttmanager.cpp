#include "mqttmanager.h"
#include "helper.h"
#include "waterlevelmanager.h"
#include <cstring> // Phase 3.5: for memcpy and strcmp in hot-path optimization

namespace
{
bool tryParseBool(const char *value, bool &out)
{
  if (strcmp(value, "true") == 0 || strcmp(value, "1") == 0 ||
      strcmp(value, "on") == 0)
  {
    out = true;
    return true;
  }

  if (strcmp(value, "false") == 0 || strcmp(value, "0") == 0 ||
      strcmp(value, "off") == 0)
  {
    out = false;
    return true;
  }

  return false;
}
} // namespace

// Initialize the static instance pointer
MqttManager *MqttManager::_instance = nullptr;

MqttManager::MqttManager()
{
  // Store the instance pointer
  _instance = this;
  _pubSubClient.setClient(_wifiClient);
}

const char *MqttManager::sanitizeMqttServer(const char *mqttServer)
{
  if (mqttServer == nullptr)
  {
    _mqttServerSanitized[0] = '\0';
    return _mqttServerSanitized;
  }

  const char *hostStart = mqttServer;
  if (strncmp(hostStart, "http://", 7) == 0)
  {
    hostStart += 7;
  }
  else if (strncmp(hostStart, "https://", 8) == 0)
  {
    hostStart += 8;
  }
  else if (strncmp(hostStart, "mqtt://", 7) == 0)
  {
    hostStart += 7;
  }

  int i = 0;
  while (hostStart[i] != '\0' && hostStart[i] != '/' &&
         i < MQTT_SERVER_MAX_LEN - 1)
  {
    _mqttServerSanitized[i] = hostStart[i];
    i++;
  }
  _mqttServerSanitized[i] = '\0';

  if (strcmp(_mqttServerSanitized, mqttServer) != 0)
  {
    Trace::log(TraceLevel::INFO,
               "MqttManager::setup | Normalized MQTT server from '" +
                   String(mqttServer) + "' to '" +
                   String(_mqttServerSanitized) + "'");
  }

  return _mqttServerSanitized;
}

const char *MqttManager::getMqttClientId()
{
  if (_mqttClientId[0] != '\0')
  {
    return _mqttClientId;
  }

  uint64_t mac = ESP.getEfuseMac();
  unsigned int suffix = static_cast<unsigned int>(mac & 0xFFFFFF);
  snprintf(_mqttClientId, sizeof(_mqttClientId), "GC-%06X", suffix);

  Trace::log(TraceLevel::INFO, "MqttManager::setup | Using MQTT client ID: " +
                                   String(_mqttClientId));

  return _mqttClientId;
}

void MqttManager::configure(const IrrigationConfig &config)
{
  _irrigationConfig = config;
}

void MqttManager::setup(const char *mqttServer, int mqttPort,
                        const char *mqttUser, const char *mqttPassword,
                        const char *clientName)
{
  _mqttServer = sanitizeMqttServer(mqttServer);
  _mqttClientId[0] = '\0';
  _mqttPort = mqttPort;
  _mqttUser = mqttUser;
  _mqttPassword = mqttPassword;
  _clientName = clientName;
  _pubSubClient.setServer(_mqttServer, _mqttPort);
  _pubSubClient.setCallback(staticMqttCallback);

  // reconnect(); // Attempt to connect to MQTT broker
  Trace::log(TraceLevel::INFO, "MqttManager setup complete.");
}

void MqttManager::initPublish()
{
  if (_waterLevelManager != nullptr)
  {
    publish(GetLowWaterLockoutEnabledTopic().c_str(),
            _waterLevelManager->isLowWaterLockoutEnabled() ? "true" : "false");
  }

  char remainingTimeBuffer[8];
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    publish(_irrigationZones[i]->getMqttTopicForRelay().c_str(),
            _irrigationZones[i]->getRelayState() ? "true" : "false");

    _irrigationZones[i]->getRemainingTimeAsString(remainingTimeBuffer,
                                                  sizeof(remainingTimeBuffer));
    publish(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str(),
            remainingTimeBuffer);

    publish(_irrigationZones[i]->getMqttTopicForSwButton().c_str(),
            _irrigationZones[i]->getBtnState() ? "true" : "false");

    int durationTimeMs = _irrigationZones[i]->getDurationTime();
    int durationTimeSeconds = durationTimeMs / 1000;
    publish(_irrigationZones[i]->getMqttTopicForDurationTime().c_str(),
            String(durationTimeSeconds).c_str());
  }
}

// Static callback that will be called by PubSubClient
void MqttManager::staticMqttCallback(char *topic, byte *payload,
                                     unsigned int length)
{
  Trace::log(TraceLevel::DEBUG,
             "staticMqttCallback called. Message arrived for topic [" +
                 String(topic) + "]");

  // Forward to the instance method if instance exists
  if (_instance)
  {
    _instance->instanceMqttCallback(topic, payload, length);
  }
}

// Instance callback that processes the actual MQTT message
void MqttManager::instanceMqttCallback(char *topic, byte *payload,
                                       unsigned int length)
{
  // Phase 3.5 optimization: Use fixed char buffer instead of String
  // concatenation in hot path This reduces heap fragmentation on ESP32 for
  // high-frequency MQTT messages
  char message[256];
  int copyLength =
      (length < (int)sizeof(message) - 1) ? length : (int)sizeof(message) - 1;
  memcpy(message, payload, copyLength);
  message[copyLength] = '\0';

  String incomingTopic = String(topic);
  Trace::log(TraceLevel::INFO, "MQTT message arrived [" + incomingTopic +
                                   "] payload: " + String(message));

  if (incomingTopic == GetFactoryResetTopic())
  {
    bool trigger = false;
    if (tryParseBool(message, trigger) && trigger)
    {
      triggerFactoryReset();
    }
    return;
  }

  if (incomingTopic == GetResetDurationsTopic())
  {
    bool trigger = false;
    if (tryParseBool(message, trigger) && trigger)
    {
      const int defaultMs = _irrigationConfig.defaultDurationMs;
      const int defaultSeconds = defaultMs / 1000;
      for (int i = 0; i < _numIrrigationZones; i++)
      {
        _irrigationZones[i]->setDurationTime(defaultMs, i);
        publish(_irrigationZones[i]->getMqttTopicForDurationTime().c_str(),
                String(defaultSeconds).c_str());
      }
      Trace::log(TraceLevel::INFO,
                 "All irrigation durations reset to default: " +
                     String(defaultSeconds) + "s");
      _resetDurationsPending = true;
      _resetDurationsClearAt = millis() + 1000;
    }
    return;
  }

  if (_waterLevelManager != nullptr &&
      (incomingTopic == GetLowWaterLockoutEnabledTopic() ||
       incomingTopic == GetLowWaterLockoutEnabledSetTopic()))
  {
    bool requestedState = false;
    if (tryParseBool(message, requestedState))
    {
      _waterLevelManager->setLowWaterLockoutEnabled(requestedState);
      publish(GetLowWaterLockoutEnabledTopic().c_str(),
              requestedState ? "true" : "false");

      Trace::log(TraceLevel::INFO,
                 "Updated lowWaterLockoutEnabled to " +
                     String(requestedState ? "true" : "false"));
    }
    else
    {
      Trace::log(TraceLevel::ERROR,
                 "Invalid payload for lowWaterLockoutEnabled: " +
                     String(message));
    }
    return;
  }

  // Check if the message is for the software button of any irrigation zone
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    String swButtonTopic = _irrigationZones[i]->getMqttTopicForSwButton();
    if (incomingTopic == swButtonTopic ||
        incomingTopic.endsWith("/swBtn") &&
            incomingTopic.indexOf(_irrigationZones[i]->getMqttTopicForZone()) >=
                0)
    {
      Trace::log(TraceLevel::INFO,
                 "Processing software button message for zone " + String(i) +
                     ": " + String(message));
      _irrigationZones[i]->synchronizeButtonStates(strcmp(message, "true") ==
                                                   0);
      break; // Exit the loop after processing the message
    }

    String durationTopic = _irrigationZones[i]->getMqttTopicForDurationTime();
    if ((incomingTopic.endsWith("/durationTime") ||
         incomingTopic.endsWith("/durationTime/set")) &&
        incomingTopic.indexOf(_irrigationZones[i]->getMqttTopicForZone()) >= 0)
    {
      int durationTimeSeconds =
          atoi(message); // Use atoi instead of String::toInt()
      int durationTimeMs =
          durationTimeSeconds * 1000; // Convert seconds to milliseconds
      if (durationTimeMs > 0 &&
          durationTimeMs <= (int)_irrigationConfig.maxDurationMs)
      {
        // Update to use new method with zone index for storage
        _irrigationZones[i]->setDurationTime(durationTimeMs, i);
        publish(durationTopic.c_str(), String(durationTimeSeconds).c_str());
        Trace::log(TraceLevel::INFO,
                   "Updated duration time for zone " + String(i) + ": " +
                       String(durationTimeSeconds) + " seconds (" +
                       String(durationTimeMs) + " ms)");
      }
      else
      {
        // Invalid duration time, reset to default
        _irrigationZones[i]->setDurationTime(
            _irrigationConfig.defaultDurationMs, i);
        Trace::log(TraceLevel::ERROR,
                   "Invalid duration time received for zone " + String(i) +
                       ": " + String(durationTimeSeconds) + " seconds");
      }
      break; // Exit the loop after processing the message
    }
  }
}

void MqttManager::loop()
{
  if (_resetDurationsPending && (long)(millis() - _resetDurationsClearAt) >= 0)
  {
    _resetDurationsPending = false;
    publish(GetResetDurationsTopic().c_str(), "false");
    Trace::log(TraceLevel::INFO, "resetDurations auto-cleared to false");
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    // Can't connect to MQTT without WiFi
    forceDisconnect();
    return;
  }

  switch (_sessionManager.state())
  {
  case MqttSessionManager::MQTT_DISCONNECTED_STATE:
    if (_connectRequested && _sessionManager.shouldAttemptConnect(millis()))
    {
      reconnect();
    }
    break;

  case MqttSessionManager::MQTT_CONNECTED_STATE:
    if (!_pubSubClient.connected())
    {
      Trace::log(TraceLevel::INFO, "MQTT connection lost");
      _sessionManager.onConnectionLost();
    }
    else
    {
      _pubSubClient.loop(); // Process incoming messages and maintain connection
    }
    break;

  case MqttSessionManager::MQTT_CONNECTING_STATE:
    // Connection attempts are synchronous in this implementation.
    break;
  }
}

void MqttManager::reconnect()
{
  // Single connection attempt
  Trace::log(TraceLevel::INFO,
             "MqttManager::reconnect | Attempting MQTT connection...");
  Trace::log(TraceLevel::INFO,
             "MqttManager::reconnect | Server=" + String(_mqttServer) + ":" +
                 String(_mqttPort) + ", ClientId=" + String(getMqttClientId()));
  _sessionManager.onConnectAttemptStarted();

  // Phase 5.2: Last Will Testament (LWT) support for system stability
  // If device disconnects unexpectedly, broker will automatically publish
  // "offline" to status topic
  if (_pubSubClient.connect(getMqttClientId(), _mqttUser, _mqttPassword,
                            getLwtTopic(), 1, true, getLwtOfflinePayload()))
  {
    Trace::log(TraceLevel::INFO, "MqttManager::reconnect | MQTT connected");
    _sessionManager.onConnectSuccess();

    // Publish "online" status immediately after successful connection
    publishOnlineStatus();

    // Publish initial state of all irrigation zones
    initPublish();

    // Resubscribe to all topics for irrigation zones
    subscribeIrrigationZones();
  }
  else
  {
    _sessionManager.onConnectFailure();
    Trace::log(
        TraceLevel::ERROR,
        "MQTT connection failed, rc=" + String(_pubSubClient.state()) +
            ", attempts: " + String(_sessionManager.reconnectAttempts()));

    if (_sessionManager.reconnectAttempts() == 0)
    {
      Trace::log(
          TraceLevel::ERROR,
          "Maximum MQTT reconnection attempts reached, will try again later");
    }
  }
}

void MqttManager::publish(const char *topic, const char *payload)
{
  if (_sessionManager.isConnected())
  {
    _pubSubClient.publish(topic, payload);
  }
  else
  {
    Trace::log(TraceLevel::ERROR, "Cannot publish to MQTT - not connected");
  }
}

void MqttManager::subscribeIrrigationZones()
{
  Trace::log(TraceLevel::INFO, "MqttManager::subscribeIrrigationZones | "
                               "Subscribing to irrigation zones...");

  subscribe(GetLowWaterLockoutEnabledTopic().c_str());
  subscribe(GetLowWaterLockoutEnabledSetTopic().c_str());
  subscribe(GetResetDurationsTopic().c_str());
  subscribe(GetFactoryResetTopic().c_str());

  // Subscribe to all irrigation zones
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    subscribe(_irrigationZones[i]->getMqttTopicForRelay().c_str());
    subscribe(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str());
    subscribe(_irrigationZones[i]->getMqttTopicForSwButton().c_str());
    subscribe(_irrigationZones[i]->getMqttTopicForDurationTime().c_str());
    String durationSetTopic =
        _irrigationZones[i]->getMqttTopicForDurationTime() + "/set";
    subscribe(durationSetTopic.c_str());
  }
}

void MqttManager::subscribe(const char *topic)
{
  if (_sessionManager.isConnected())
  {
    _pubSubClient.subscribe(topic);
    Trace::log(TraceLevel::INFO, "Subscribed to: " + String(topic));
  }
  else
  {
    Trace::log(TraceLevel::ERROR, "Cannot subscribe to MQTT - not connected");
  }
}

void MqttManager::addIrrigationZone(IrrigationZone *zone)
{
  if (_numIrrigationZones < MAX_IRRIGATION_ZONES)
  {
    _irrigationZones[_numIrrigationZones] = zone;
    _numIrrigationZones++;
    Trace::log(TraceLevel::INFO,
               "MqttManager::addIrrigationZone | Added irrigation zone: " +
                   zone->getMqttTopicForZone());
  }
  else
  {
    Trace::log(TraceLevel::ERROR, "MqttManager::addIrrigationZone | Maximum "
                                  "number of irrigation zones reached.");
  }
}

bool MqttManager::isConnected()
{
  return _sessionManager.isConnected();
}

void MqttManager::publishAllIrrigationZones()
{
  if (isConnected())
  {
    // Phase 3.5 optimization: Use stack-allocated char buffer instead of String
    // objects This reduces heap fragmentation for frequent MQTT publish
    // operations
    char remainingTimeBuffer[8];

    for (int i = 0; i < _numIrrigationZones; i++)
    {
      publish(_irrigationZones[i]->getMqttTopicForRelay().c_str(),
              _irrigationZones[i]->getRelayState() ? "true" : "false");

      // Use optimized buffer method instead of String conversion
      _irrigationZones[i]->getRemainingTimeAsString(
          remainingTimeBuffer, sizeof(remainingTimeBuffer));
      publish(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str(),
              remainingTimeBuffer);

      publish(_irrigationZones[i]->getMqttTopicForSwButton().c_str(),
              _irrigationZones[i]->getBtnState() ? "true" : "false");

      int durationTimeSeconds = _irrigationZones[i]->getDurationTime() / 1000;
      publish(_irrigationZones[i]->getMqttTopicForDurationTime().c_str(),
              String(durationTimeSeconds).c_str());
    }
  }
}

void MqttManager::publishSystemStatus()
{
  if (!isConnected())
  {
    return;
  }

  // Publish online status
  publish(GetStatusTopic().c_str(), "online");

  // Publish system information
  String freeHeap = Helper::formatMemory(ESP.getFreeHeap());
  publish(GetFreeMemoryTopic().c_str(), freeHeap.c_str());

  // Publish WiFi signal strength
  String rssi = String(WiFi.RSSI());
  publish(GetWifiSignalStrengthTopic().c_str(), rssi.c_str());

  // Publish heartbeat timestamp
  String timestamp = String(millis());
  publish(GetHeartbeatTopic().c_str(), timestamp.c_str());

  // Also update uptime
  String uptime = Helper::formatUptime(millis()); // Uptime in "Xd Yh Zm" format
  publish(GetUptimeTopic().c_str(), uptime.c_str());

  Trace::log(TraceLevel::INFO, "System status published");
}

void MqttManager::disconnect()
{
  if (isConnected())
  {
    // Publish offline status before disconnecting
    publish(GetStatusTopic().c_str(), "offline");

    // Give a moment for the message to be sent
    delay(100);

    Trace::log(TraceLevel::INFO, "MQTT disconnect requested gracefully");
  }

  forceDisconnect();
}

void MqttManager::requestConnect()
{
  if (!_connectRequested)
  {
    Trace::log(TraceLevel::INFO, "MQTT connect requested");
  }

  _connectRequested = true;
}

void MqttManager::forceDisconnect()
{
  _connectRequested = false;

  if (_pubSubClient.connected())
  {
    _pubSubClient.disconnect();
  }

  _sessionManager.forceDisconnect();
}

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
}

void MqttManager::setWaterLevelManager(WaterLevelManager *waterLevelManager)
{
  _waterLevelManager = waterLevelManager;
}

void MqttManager::triggerFactoryReset()
{
  Trace::log(TraceLevel::ERROR, "!!! FACTORY RESET TRIGGERED VIA MQTT !!!");

  // Reset all zones in memory and stop any active irrigation
  const int defaultMs = _irrigationConfig.defaultDurationMs;
  const int defaultSeconds = defaultMs / 1000;
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    _irrigationZones[i]->synchronizeButtonStates(false);
    _irrigationZones[i]->switchRelay(false);
    _irrigationZones[i]->resetTimer();
    _irrigationZones[i]->setDurationTime(defaultMs, i);
  }

  // Reset lockout config to default
  if (_waterLevelManager != nullptr)
  {
    _waterLevelManager->setLowWaterLockoutEnabled(
        WATER_LEVEL_LOW_WATER_LOCKOUT_ENABLED);
  }

  // Persist all defaults to NVS
  StorageManager::getInstance().factoryReset(defaultMs);

  // Publish reset state to all topics
  if (_sessionManager.isConnected())
  {
    for (int i = 0; i < _numIrrigationZones; i++)
    {
      publish(_irrigationZones[i]->getMqttTopicForRelay().c_str(), "false");
      publish(_irrigationZones[i]->getMqttTopicForSwButton().c_str(), "false");
      publish(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str(),
              "00:00");
      publish(_irrigationZones[i]->getMqttTopicForDurationTime().c_str(),
              String(defaultSeconds).c_str());
    }

    if (_waterLevelManager != nullptr)
    {
      publish(GetLowWaterLockoutEnabledTopic().c_str(),
              WATER_LEVEL_LOW_WATER_LOCKOUT_ENABLED ? "true" : "false");
    }

    publish(GetFactoryResetStatusTopic().c_str(), "complete");
    Trace::log(TraceLevel::INFO, "Factory reset complete, status published");
  }
}