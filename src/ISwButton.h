// ISwButton.h

#ifndef ISW_BUTTON_H
#define ISW_BUTTON_H

#include <Arduino.h>
#include "IMqttClient.h"

class ISwButton
{
  public:
  virtual void setup(IMqttClient* mqttClient, int btnId) = 0;
  virtual bool getState() = 0;
  virtual void setState(bool state) = 0;
};

#endif // ISW_BUTTON_H