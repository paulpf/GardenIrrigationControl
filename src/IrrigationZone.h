// IrrigationZone.h
#ifndef IRRIGATIONZONE_H
#define IRRIGATIONZONE_H

#include <Arduino.h>
#include "FunctionalInterrupt.h"

class IrrigationZone 
{
public:
  IrrigationZone();
  ~IrrigationZone();
  void setup();
  bool getRelaisState(int relayId);
  void setSwBtnState(bool btnState);
  void setHwBtnState(bool btnState);
  void setDuration(int duration);

private:
  int _relayId;
  bool _swBtnState;
  bool _hwBtnState;
  unsigned long _duration;
  unsigned long _startTime;
};

#endif