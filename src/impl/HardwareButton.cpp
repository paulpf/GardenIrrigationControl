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

static void IRAM_ATTR handleButtonReleased() 
{
  if (activeInstance) 
  {
    activeInstance->buttonReleased();
  }
}

HardwareButton::HardwareButton(int gpioPin)
{
  _gpioPin = gpioPin;
  _pressed = false;
  
  // Store instance pointer for the interrupt handlers
  activeInstance = this;
  
  pinMode(_gpioPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_gpioPin), handleButtonPressed, RISING);
  attachInterrupt(digitalPinToInterrupt(_gpioPin), handleButtonReleased, FALLING);
}

void IRAM_ATTR HardwareButton::buttonPressed() 
{ 
  _pressed = true;
}

void IRAM_ATTR HardwareButton::buttonReleased() 
{ 
  _pressed = false;
}

bool HardwareButton::isPressed()
{
  return _pressed;
}