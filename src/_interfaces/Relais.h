// Relais.h
#ifndef RELAIS_H
#define RELAIS_H

#include <Arduino.h>

class Relais
{
public:
  Relais();
  ~Relais();
  void setup(int gpioPin);
  void set(bool state);

private:
  int _gpioPin;
  bool _lastState;
};

#endif // RELAIS_H