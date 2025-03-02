// HardwareButton.cpp

//#define DEBUG

#include "./_interfaces/HardwareButton.h"

// Constructor
HardwareButton::HardwareButton()
{
}

// Destructor
HardwareButton::~HardwareButton()
{
}

// Setup the hardware button
void HardwareButton::setup(int gpioPin)
{
  _gpioPin = gpioPin;
  _pressed = false;
  _stateBefore = false;
  pinMode(_gpioPin, INPUT_PULLDOWN);
}

// Check if the button is pressed
bool HardwareButton::isPressed()
{
  bool stateNow = digitalRead(_gpioPin);
  if (stateNow != _stateBefore)
  {
    _pressed = _stateBefore = stateNow;
    #ifdef DEBUG
    Serial.println(_pressed ? "Button is pressed" : "Button is not pressed");
    #endif
  }
  return _pressed;
}