// HwButton.cpp

#include <Arduino.h>
#include "HwButton.h"
#include "FunctionalInterrupt.h"
#include "Trace.h"

HwButton::HwButton()
{
}

HwButton::~HwButton()
{
}

void HwButton::setup(int gpioPin)
{
  _gpioPin = gpioPin;
  _lastDebounceTime = 0;
  _buttonPressed = false;
  pinMode(gpioPin, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(_gpioPin), std::bind(&HwButton::OnHwBtnPressed, this), RISING);
}

bool HwButton::getState()
{
  bool state = _buttonPressed;
  _buttonPressed = false;
  return state;
}

void IRAM_ATTR HwButton::OnHwBtnPressed() 
{
  Trace::log("Interrupt triggered");
  unsigned long now = millis();
  if (now - _lastDebounceTime > _debounceDelay) 
  {
    _lastDebounceTime = now;
    _buttonPressed = true;
    Trace::log("Button pressed");
  }
}
