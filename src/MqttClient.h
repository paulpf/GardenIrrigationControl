// MqttClient.h

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "Data.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include "ISwButton.h"
#include "ITimer.h"
#include "IMqttClient.h"
#include <unordered_map>

class MqttClient : public IMqttClient
{
public:
  MqttClient();
  ~MqttClient();
  static MqttClient *instance;    
  static void mqttCallback(char *topic, byte *message, unsigned int length);
  
  void setup(WiFiClient *wifiClient, String deviceName);
  void loop();
  void registerSwButton(ISwButton *swButton, int btnId);
  void registerTimer(ITimer *timer, int tmrid);
  void reconnectMqtt();

private:
  WiFiClient *_wifiClient;
  String _deviceName;
  PubSubClient _pubSubClient;
  static const int MAX_SW_BUTTONS = 10;
  static const int MAX_HW_BUTTONS = 10;
  std::unordered_map<int, ISwButton*> _registeredSwButtons;
  std::unordered_map<int, ITimer*> _registeredTimers;
};

#endif // MQTT_CLIENT_H
