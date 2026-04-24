#pragma once
#include "iwaterlevelsensorreader.h"

class FakeWaterLevelSensor : public IWaterLevelSensorReader
{
public:
  int rawValue = 0;

  void setup() override {}
  int readRaw() override { return rawValue; }
};
