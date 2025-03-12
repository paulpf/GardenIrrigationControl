// Relais.cpp

#include "Relais.h"
#include <Arduino.h>
#include "Trace.h"

Relais::Relais()
{
}

Relais::~Relais()
{
}

void Relais::setup(int gpioPin)
{
  _gpioPin = gpioPin;
  pinMode(gpioPin, OUTPUT);
  switchRelayOFF();
}

void Relais::setState(bool state)
{
  bool currentState = getState();
  if (state != currentState)
  {
    if (state)
    {
        switchRelayON();
    }
    else
    {
        switchRelayOFF(); 
    }
  }
}

bool Relais::getState()
{
  int currentState = getState() ? HIGH : LOW;
  Trace::log("Relais state: " + String(currentState));
  return currentState;
}

void Relais::switchRelayON()
{
  Trace::log("Switching relay ON");
  digitalWrite(_gpioPin, LOW);
}

void Relais::switchRelayOFF()
{
  Trace::log("Switching relay OFF");
  digitalWrite(_gpioPin, HIGH);
}