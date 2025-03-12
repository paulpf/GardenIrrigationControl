// IHwButton.h

#ifndef IHwButton_H
#define IHwButton_H

#include <Arduino.h>

class IHwButton
{
  public:
  virtual void setup(int pin) = 0;
  virtual bool getState() = 0;
};

#endif // IHwButton_H