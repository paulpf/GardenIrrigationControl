// IrrigationZone.h
#ifndef IRRIGATIONZONE_H
#define IRRIGATIONZONE_H

#pragma once

#include <Arduino.h>
#include "HardwareButton.h"
#include "Relais.h"

class IrrigationZone
{
public:
  IrrigationZone(String name, int hwButtonGpioPin, String swButtonAddress, int relaisGpioPin);
  ~IrrigationZone();
  void readInputs();
  void processLogic();
  void writeOutputs();

private:
  String _name;
  HardwareButton _hwButton;
  String _swButtonAddress;
  Relais _relais;

  bool _hwButtonIsPressed;
};

#endif // IRRIGATIONZONE_H