// HardwareButton.h

#ifndef HARDWAREBUTTON_H
#define HARDWAREBUTTON_H

#include <Arduino.h>

class HardwareButton
{
public:
  HardwareButton();
  ~HardwareButton();
  void setup(int gpioPin);
  bool isPressed();

private:
  int _gpioPin;
  bool _pressed;
  bool _stateBefore;
};

#endif // HARDWAREBUTTON_H