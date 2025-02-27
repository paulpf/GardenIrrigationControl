// IrrigationZone.h
#ifndef IRRIGATIONZONE_H
#define IRRIGATIONZONE_H

#pragma once

#include <Arduino.h>
#include "HardwareButton.h"

class IrrigationZone
{
public:
  IrrigationZone(String name, int hwButtonGpioPin, String swButtonAddress);
  ~IrrigationZone();

private:
  String _name;
  HardwareButton _hwButton;
  String _swButtonAddress;
};

#endif // IRRIGATIONZONE_H