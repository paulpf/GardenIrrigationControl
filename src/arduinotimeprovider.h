#ifndef ARDUINOTIMEPROVIDER_H
#define ARDUINOTIMEPROVIDER_H

#include <Arduino.h>

#include "itimeprovider.h"

class ArduinoTimeProvider : public ITimeProvider
{
public:
  unsigned long millis() const override
  {
    return ::millis();
  }

  void delayMs(unsigned long ms) override
  {
    ::delay(ms);
  }
};

#endif // ARDUINOTIMEPROVIDER_H
