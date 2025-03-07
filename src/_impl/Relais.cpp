// Relais.cpp

#include "./_interfaces/Relais.h"

// Constructor
Relais::Relais()
{
}

// Destructor
Relais::~Relais()
{
}

// Setup the relais
void Relais::setup(int gpioPin)
{
  _gpioPin = gpioPin;
  pinMode(_gpioPin, OUTPUT);
  set(false);
}

// Update the relais
void Relais::set(bool state)
{
  if (_lastState == state)
  {
    return;
  }
  Serial.print("Relais: Setting state to ");
  Serial.println(state);
  digitalWrite(_gpioPin, state);
  _lastState = state;
}