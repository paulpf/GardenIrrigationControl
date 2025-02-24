// Valve.h
#ifndef VALVE_H
#define VALVE_H

#pragma once

#include <Arduino.h>

enum ValveState
{
  OPEN,
  CLOSE
};

class Valve
{
public:
  Valve(String name, int GPIOChannel);
  ~Valve();
  ValveState getValveState();
  void setValveState(ValveState state);

private:
  String name;
  int GPIOChannel;
  ValveState state;
};

#endif // VALVE_H