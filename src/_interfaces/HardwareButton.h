// HardwareButton.h
#ifndef HARDWARE_BUTTON_H
#define HARDWARE_BUTTON_H

#include <Arduino.h>

class HardwareButton
{
  public:
    HardwareButton(int gpioPin);
    bool isPressed();
    
    // Make these methods public so they can be called by interrupt handlers
    void IRAM_ATTR buttonPressed();
    void IRAM_ATTR buttonReleased();
    
  private:
    int _gpioPin;
    volatile bool _pressed;
};

#endif // HARDWARE_BUTTON_H