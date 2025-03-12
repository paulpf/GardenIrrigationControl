// IMqttClient.h

#ifndef IMQTT_CLIENT_H
#define IMQTT_CLIENT_H

#include <Arduino.h>
#include <WiFiClient.h>
#include <unordered_map>

// Forward declarations
class ISwButton;
class ITimer;

class IMqttClient
{
public:
  virtual void setup(WiFiClient *wifiClient, String deviceName) = 0;
  virtual void loop() = 0;
  virtual void registerSwButton(ISwButton *swButton, int btnId) = 0;
  virtual void registerTimer(ITimer *timer, int tmrid) = 0;
  virtual void reconnectMqtt() = 0;
};

#endif // IMQTT_CLIENT_H