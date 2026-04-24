#pragma once

#include "ArduinoFake.h"

class WiFiClass
{
public:
  String macAddress() const
  {
    return String("00:11:22:33:44:55");
  }
};

static WiFiClass WiFi;
