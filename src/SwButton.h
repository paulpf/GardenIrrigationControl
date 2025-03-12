// SwButton.h

#ifndef SW_BUTTON_H
#define SW_BUTTON_H

#include <Arduino.h>
#include "IMqttClient.h"
#include "ISwButton.h"

class SwButton : public ISwButton
{
  public:
    SwButton();
    ~SwButton();
    void setup(IMqttClient* mqttClient, int btnId);
    bool getState();
    void setState(bool state);
  private:
    IMqttClient* _mqttClient;
    int _btnId;
    bool _lastState;
};

#endif