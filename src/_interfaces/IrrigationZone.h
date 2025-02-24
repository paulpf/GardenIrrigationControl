// IrrigationZone.h
#ifndef IRRIGATIONZONE_H
#define IRRIGATIONZONE_H

#pragma once

#include <Arduino.h>
#include "Valve.h"

class IrrigationZone
{
public:
  IrrigationZone(String name, int gpioChannel);
  ~IrrigationZone();
  int getDuration();
  void setDuration(int duration);
  void start();
  void stop();

private:
  Valve *valve;
  bool isActive = false;
  String name;
  int duration;
  int timeLeft;
  unsigned long previousMillis;
  const unsigned long updateInterval = 1000; // Update publish every second
};

#endif // IRRIGATIONZONE_H