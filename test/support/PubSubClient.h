#pragma once

#include "Arduino.h"
#include "WiFi.h"

typedef unsigned char byte;

class PubSubClient
{
public:
  PubSubClient() {}

  explicit PubSubClient(WiFiClient &) {}

  void setClient(WiFiClient &) {}

  void setServer(const char *, int) {}

  void setCallback(void (*)(char *, byte *, unsigned int)) {}

  bool connect(const char *, const char *, const char *, const char *, int, bool,
               const char *)
  {
    return true;
  }

  bool connected() const
  {
    return true;
  }

  bool publish(const char *, const char *, bool = false)
  {
    return true;
  }

  bool subscribe(const char *)
  {
    return true;
  }

  void loop() {}

  void disconnect() {}
};
