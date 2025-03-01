// Relais.h
#ifndef RELAIS_H
#define RELAIS_H

#pragma once

#include <Arduino.h>

enum RelaisState
{
  OPEN,
  CLOSE
};

class Relais
{
public:
  Relais(int gpioChannel);
  ~Relais();
  RelaisState getRelaisState();
  void setRelaisState(RelaisState state);

private:
  int _gpioChannel;
  RelaisState _state;
};

#endif //RELAIS_H