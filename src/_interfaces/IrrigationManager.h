// IrrigationManager.h
#ifndef IRRIGATIONMANAGER_H
#define IRRIGATIONMANAGER_H

#pragma once

#include <Arduino.h>
#include "IrrigationZone.h"

class IrrigationManager
{
public:
  IrrigationManager();
  ~IrrigationManager();
  void setup();
  void updateZone(int zoneIndex);
  void stopZone(int zoneIndex);

private:
  IrrigationZone *irrigationZones[8];
  Valve *drainageValve;
  void addZone(IrrigationZone *irrigationZone);
  void removeZone(IrrigationZone *irrigationZone);
};

#endif // IRRIGATIONMANAGER_H