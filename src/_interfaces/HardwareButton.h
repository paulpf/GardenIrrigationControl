// HardwareButton.h

#include <Arduino.h>

#ifndef HARDWAREBUTTON_H
#define HARDWAREBUTTON_H

class HardwareButton
{
public:
    HardwareButton();
    ~HardwareButton();

    void setup(int gpioPin);
    bool isPressed();

private:
    int _gpioPin;
};

#endif // HARDWAREBUTTON_H