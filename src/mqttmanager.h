// mqttmanager.h
#ifndef MQTTMANAGER_H
#define MQTTMANAGER_H

#include "globaldefines.h"
#include <PubSubClient.h>
#include "irrigationZone.h"

class MqttManager {

public:
    MqttManager();
    void setup(const char* mqttServer, int mqttPort, const char* mqttUser, const char* mqttPassword, const char* clientName);
    void loop();
    void initPublish();
    void publish(const char* topic, const char* payload);
    void subscribe(const char* topic);
    void addIrrigationZone(IrrigationZone *zone);
    void subscribeIrrigationZones();
    bool isConnected();
    void publishAllIrrigationZones();

    // Make this public and static so it can be used as a callback
    static void staticMqttCallback(char* topic, byte* payload, unsigned int length);

private:
    // Instance callback that will be called by the static callback
  void instanceMqttCallback(char *topic, byte *payload, unsigned int length);
  void handleTopicForSwButton(char *topic, int i, String &message, int &retFlag);
  void reconnect();

  bool _blockPublish; // Flag to block publish if needed

  // Static pointer to the MqttManager instance
  static MqttManager *_instance;

  const char *_mqttServer;
  int _mqttPort;
  const char *_mqttUser;
  const char *_mqttPassword;
  const char *_clientName;

  WiFiClient _wifiClient;
  PubSubClient _pubSubClient;

  // MQTT state management
  enum MqttState
  {
    MQTT_DISCONNECTED_STATE,
    MQTT_CONNECTING_STATE,
    MQTT_CONNECTED_STATE
  };
  MqttState _mqttState = MQTT_DISCONNECTED_STATE;
  unsigned long _lastMqttAttemptMillis = 0;
  const unsigned long _mqttRetryInterval = 5000; // Wait 5 seconds between connection attempts
  int _mqttReconnectAttempts = 0;
  const int _maxMqttReconnectAttempts = 5;

  // Array of irrigation zones
  IrrigationZone *_irrigationZones[MAX_IRRIGATION_ZONES];
  int _numIrrigationZones = 0; // Number of irrigation zones
};

#endif // MQTTMANAGER_H