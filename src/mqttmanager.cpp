#include "mqttmanager.h"
#include "dht11manager.h"
#include "helper.h"
#include <cstring>  // Phase 3.5: for memcpy and strcmp in hot-path optimization

// Initialize the static instance pointer
MqttManager *MqttManager::_instance = nullptr;

MqttManager::MqttManager()
{
  // Store the instance pointer
  _instance = this;
  _pubSubClient.setClient(_wifiClient);
  _dht11Manager = nullptr; // Initialize DHT11 manager pointer
}

void MqttManager::setup(const char *mqttServer, int mqttPort,
                        const char *mqttUser, const char *mqttPassword,
                        const char *clientName)
{
  _mqttServer = mqttServer;
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
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    // Set initial state on mqtt for relay as false
    // This is necessary to ensure the relay state is known on startup
    // and to avoid false positives in the UI
    publish(_irrigationZones[i]->getMqttTopicForRelay().c_str(), "false");

    // Convert milliseconds to minutes for MQTT publishing
    int durationTimeMs = _irrigationZones[i]->getDurationTime();
    int durationTimeMinutes = durationTimeMs / (60 * 1000);
    publish(_irrigationZones[i]->getMqttTopicForDurationTime().c_str(),
            String(durationTimeMinutes).c_str());
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
  // Phase 3.5 optimization: Use fixed char buffer instead of String concatenation in hot path
  // This reduces heap fragmentation on ESP32 for high-frequency MQTT messages
  char message[256];
  int copyLength = (length < (int)sizeof(message) - 1) ? length : (int)sizeof(message) - 1;
  memcpy(message, payload, copyLength);
  message[copyLength] = '\0';

  Trace::log(TraceLevel::DEBUG,
             "Message arrived [" + String(topic) + "] Message: " + String(message));

  // Check if the message is for the software button of any irrigation zone
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    if (String(topic).startsWith(
            _irrigationZones[i]->getMqttTopicForSwButton()))
    {
      Trace::log(TraceLevel::INFO,
                 "Processing software button message for zone " + String(i) +
                     ": " + String(message));
      _irrigationZones[i]->synchronizeButtonStates(strcmp(message, "true") == 0);
      break; // Exit the loop after processing the message
    }

    if (String(topic).startsWith(
            _irrigationZones[i]->getMqttTopicForDurationTime()))
    {
      int durationTimeMinutes = atoi(message);  // Use atoi instead of String::toInt()
      int durationTimeMs =
          durationTimeMinutes * 60 * 1000; // Convert minutes to milliseconds
      if (durationTimeMs > 0 && durationTimeMs <= MAX_DURATION_TIME)
      {
        // Update to use new method with zone index for storage
        _irrigationZones[i]->setDurationTime(durationTimeMs, i);
        Trace::log(TraceLevel::DEBUG,
                   "Updated duration time for zone " + String(i) + ": " +
                       String(durationTimeMinutes) + " minutes (" +
                       String(durationTimeMs) + " ms)");
      }
      else
      {
        // Invalid duration time, reset to default
        _irrigationZones[i]->setDurationTime(DEFAULT_DURATION_TIME, i);
        Trace::log(TraceLevel::ERROR,
                   "Invalid duration time received for zone " + String(i) +
                       ": " + String(durationTimeMinutes) + " minutes");
      }
      break;                 // Exit the loop after processing the message
    }
  }
}

void MqttManager::loop()
{
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
  _sessionManager.onConnectAttemptStarted();

  // Phase 5.2: Last Will Testament (LWT) support for system stability
  // If device disconnects unexpectedly, broker will automatically publish "offline" to status topic
  if (_pubSubClient.connect(_clientName, _mqttUser, _mqttPassword,
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
  // Subscribe to all irrigation zones
  for (int i = 0; i < _numIrrigationZones; i++)
  {
    subscribe(_irrigationZones[i]->getMqttTopicForRelay().c_str());
    subscribe(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str());
    subscribe(_irrigationZones[i]->getMqttTopicForSwButton().c_str());
    subscribe(_irrigationZones[i]->getMqttTopicForDurationTime().c_str());
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
    // Phase 3.5 optimization: Use stack-allocated char buffer instead of String objects
    // This reduces heap fragmentation for frequent MQTT publish operations
    char remainingTimeBuffer[8];
    
    for (int i = 0; i < _numIrrigationZones; i++)
    {
      publish(_irrigationZones[i]->getMqttTopicForRelay().c_str(),
              _irrigationZones[i]->getRelayState() ? "true" : "false");
      
      // Use optimized buffer method instead of String conversion
      _irrigationZones[i]->getRemainingTimeAsString(remainingTimeBuffer, sizeof(remainingTimeBuffer));
      publish(_irrigationZones[i]->getMqttTopicForRemainingTime().c_str(),
              remainingTimeBuffer);
      
      publish(_irrigationZones[i]->getMqttTopicForSwButton().c_str(),
              _irrigationZones[i]->getBtnState() ? "true" : "false");
    }
  }
}

void MqttManager::setDht11Manager(Dht11Manager *dht11Manager)
{
  _dht11Manager = dht11Manager;
  Trace::log(TraceLevel::INFO, "DHT11 manager assigned to MQTT manager");
}

void MqttManager::publishDht11Data()
{
  if (!isConnected() || _dht11Manager == nullptr)
  {
    return;
  }

  if (_dht11Manager->isDataValid())
  {
    // Publish temperature
    String tempStr = String(_dht11Manager->getTemperature(), 1);
    publish(_dht11Manager->getMqttTopicForTemperature().c_str(),
            tempStr.c_str());

    // Publish humidity
    String humStr = String(_dht11Manager->getHumidity(), 1);
    publish(_dht11Manager->getMqttTopicForHumidity().c_str(), humStr.c_str());

    // Publish heat index
    String heatIndexStr = String(_dht11Manager->getHeatIndex(), 1);
    publish(_dht11Manager->getMqttTopicForHeatIndex().c_str(),
            heatIndexStr.c_str());

    // Publish sensor status
    publish(_dht11Manager->getMqttTopicForStatus().c_str(), "online");

    // Publish timestamp on separate topic
    String timestampTopic = _dht11Manager->getMqttTopicForStatus();
    timestampTopic.replace("/status", "/timestamp");
    publish(timestampTopic.c_str(), _dht11Manager->getTimeStamp().c_str());
  }
  else
  {
    // Publish offline status if sensor data is invalid
    publish(_dht11Manager->getMqttTopicForStatus().c_str(), "offline");
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
}
