// SwButton.cpp

#include "SwButton.h"
#include "SwButton.h"
#include "MqttClient.h"

SwButton::SwButton()
{
}

SwButton::~SwButton()
{
}

void SwButton::setup(IMqttClient* mqttClient, int btnId)
{
  _mqttClient = mqttClient;
  _mqttClient->registerSwButton(this, btnId);
  _btnId = btnId;
  _lastState = false;
}

bool SwButton::getState()
{
  return _lastState;
}

void SwButton::setState(bool state)
{
  if (state != _lastState)
  {
      _lastState = state;
  }
}