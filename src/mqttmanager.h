// mqttmanager.h
#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include "global_defines.h"
#include "irrigation_zone.h"
#include "mqttsessionmanager.h"
#include <PubSubClient.h>

// Forward declaration to avoid circular dependency
class Dht11Manager;

class MqttManager
{

public:
  MqttManager();
  void setup(const char *mqttServer, int mqttPort, const char *mqttUser,
             const char *mqttPassword, const char *clientName);
  void loop();
  void initPublish();
  void publish(const char *topic, const char *payload);
  void subscribe(const char *topic);
  void addIrrigationZone(IrrigationZone *zone);
  void subscribeIrrigationZones();
  bool isConnected();
  void publishAllIrrigationZones();

  // DHT11 sensor support
  void setDht11Manager(Dht11Manager *dht11Manager);
  void publishDht11Data();

  // System status and heartbeat functionality
  void publishSystemStatus();
  void disconnect();
  void requestConnect();
  void forceDisconnect();

  // Make this public and static so it can be used as a callback
  static void staticMqttCallback(char *topic, byte *payload,
                                 unsigned int length);

private:
  // Instance callback that will be called by the static callback
  void instanceMqttCallback(char *topic, byte *payload, unsigned int length);
  void reconnect();

  // Static pointer to the MqttManager instance
  static MqttManager *_instance;

  const char *_mqttServer;
  int _mqttPort;
  const char *_mqttUser;
  const char *_mqttPassword;
  const char *_clientName;
  bool _connectRequested = false;

  WiFiClient _wifiClient;
  PubSubClient _pubSubClient;

  // MQTT session and retry state management
  MqttSessionManager _sessionManager;
  // Array of irrigation zones
  IrrigationZone *_irrigationZones[MAX_IRRIGATION_ZONES];
  int _numIrrigationZones = 0; // Number of irrigation zones

  // DHT11 sensor manager
  Dht11Manager *_dht11Manager;

  // Combine name for system topic with _clientName after setup
  String GetStatusTopic() const
  {
    return _clientName + String("/system/status");
  }
  String GetFreeMemoryTopic() const
  {
    return _clientName + String("/system/freeMemory");
  }
  String GetUptimeTopic() const
  {
    return _clientName + String("/system/uptime");
  }
  String GetWifiSignalStrengthTopic() const
  {
    return _clientName + String("/system/wifiSignalStrength");
  }
  String GetHeartbeatTopic() const
  {
    return _clientName + String("/system/heartbeat");
  }
};

#endif // MQTTMANAGER_H