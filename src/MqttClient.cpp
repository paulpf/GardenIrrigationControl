// MqttClient.cpp


#include <Arduino.h>
#include <WiFiClient.h>
#include "MqttClient.h"
#include "ISwButton.h"
#include "ITimer.h"


#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/MqttSecret.h"
#include "../../_configs/MqttConfig.h"
#else
#include "_secrets/MqttSecret.h"
#include "_config/MqttConfig.h"
#endif

MqttClient *MqttClient::instance = nullptr;

MqttClient::MqttClient()
{
  instance = this;
}

MqttClient::~MqttClient()
{
}

void MqttClient::setup(WiFiClient *wifiClient, String deviceName)
{
  // Setup console
  Serial.begin(115200);

  this->_wifiClient = wifiClient;
  this->_deviceName = deviceName;
  _pubSubClient.setClient(*wifiClient);
  _pubSubClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  _pubSubClient.setCallback(MqttClient::instance->mqttCallback);
}

void MqttClient::loop()
{
  if (!_pubSubClient.connected())
  {
    reconnectMqtt();
  }
  _pubSubClient.loop();
}

void MqttClient::registerSwButton(ISwButton *swButton, int btnId)
{
  _registeredSwButtons[btnId] = swButton;  
}

void MqttClient::registerTimer(ITimer *timer, int tmrid)
{
  _registeredTimers[tmrid] = timer;
}

void MqttClient::reconnectMqtt()
{
  if (!_pubSubClient.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (_pubSubClient.connect(_deviceName.c_str(), MQTT_USER, MQTT_PWD))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      _pubSubClient.publish((_deviceName + "/status").c_str(), "connected");
      
      // ToDo: add all registered buttons to the MQTT client
      for (auto const& x : _registeredSwButtons)
      {
        _pubSubClient.subscribe((_deviceName + "/swBtn_Nr" + String(x.first)).c_str());
      }

      for (auto const& x : _registeredTimers)
      {
        _pubSubClient.subscribe((_deviceName + "/timer_Nr" + String(x.first)).c_str());
      }
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(_pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int extractIdFromTopic(const String& topic, const String& prefix)
{
  return topic.substring(topic.lastIndexOf(prefix) + prefix.length()).toInt();
}

// Callback function to handle incoming MQTT messages
void MqttClient::mqttCallback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char messageStr[length + 1];
  for (unsigned int i = 0; i < length; i++)
  {
    messageStr[i] = (char)message[i];
  }
  messageStr[length] = '\0';
  Serial.print(messageStr);
  Serial.println();

  String topicStr = String(topic);

  if (topicStr.startsWith(MqttClient::instance->_deviceName + "/swBtn_Nr"))
  {
    int btnId = extractIdFromTopic(topicStr, "Nr");
    bool btnState = strcmp(messageStr, "true") == 0;

    ISwButton *swButton = MqttClient::instance->_registeredSwButtons[btnId];
    if (swButton != nullptr)
    {
      swButton->setState(btnState);
    }
  }
  else if (topicStr.startsWith(MqttClient::instance->_deviceName + "/timer_Nr"))
  {
    int tmrid = extractIdFromTopic(topicStr, "Nr");
    unsigned long duration = atol(messageStr);

    ITimer *timer = MqttClient::instance->_registeredTimers[tmrid];
    if (timer != nullptr)
    {
      timer->setDuration(duration);
    }
  }
}