#pragma once

#include "ArduinoFake.h"

class WiFiClass
{
public:
  String macAddress() const
  {
    return String("00:11:22:33:44:55");
  }

  int status() const
  {
    return 3;
  }
};

class WiFiClient
{
};

static WiFiClass WiFi;

constexpr int WL_CONNECTED = 3;
typedef int WiFiEvent_t;
