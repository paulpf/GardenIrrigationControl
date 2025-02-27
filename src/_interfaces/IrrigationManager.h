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
  void readInputs();
  void processLogic();
  void writeOutputs();

private:
  IrrigationZone *irrigationZones[8];
};

#endif // IRRIGATIONMANAGER_H