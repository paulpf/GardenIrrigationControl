// HwButton.h
#ifndef HW_BUTTON_H
#define HW_BUTTON_H

#include <Arduino.h>
#include "IHwButton.h"

class HwButton : public IHwButton
{
  public:
    HwButton();
    ~HwButton();
    void setup(int gpioPin);
    void OnHwBtnPressed();
    bool getState();
  private:
    int _gpioPin;
    unsigned long _lastDebounceTime;
    const int _debounceDelay = 500; // debounce time in milliseconds
    bool _buttonPressed;
    bool _newState;
    bool _stateChanged;
};

#endif