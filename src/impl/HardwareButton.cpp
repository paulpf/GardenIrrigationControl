// HardwareButton.cpp
#include "./_interfaces/HardwareButton.h"

// Static instance pointer to track the active button
static HardwareButton* activeInstance = nullptr;

// Static interrupt handlers that dispatch to the instance
static void IRAM_ATTR handleButtonPressed() 
{
  if (activeInstance) 
  {
    activeInstance->buttonPressed();
  }
}

HardwareButton::HardwareButton(int gpioPin)
{
  _gpioPin = gpioPin;
  _pressed = false;
  
  // Store instance pointer for the interrupt handlers
  activeInstance = this;
  
  pinMode(_gpioPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(_gpioPin), handleButtonPressed, RISING);
}

void IRAM_ATTR HardwareButton::buttonPressed() 
{ 
  _pressed = !_pressed;
}

bool HardwareButton::isPressed()
{
  return _pressed;
}