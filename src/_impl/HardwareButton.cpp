// HardwareButton.cpp

#define DEBUG

#include "./_interfaces/HardwareButton.h"
#include <Arduino.h>

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
  pinMode(_gpioPin, INPUT_PULLDOWN);
}


// Check if the button is pressed
bool HardwareButton::isPressed()
{
  return digitalRead(_gpioPin);
}