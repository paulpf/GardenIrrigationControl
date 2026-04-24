#ifndef ESP32WATERLEVELSENSOR_H
#define ESP32WATERLEVELSENSOR_H

#include "iwaterlevelsensorreader.h"

class Esp32WaterLevelSensor : public IWaterLevelSensorReader
{
public:
  void setup() override;
  int readRaw() override;
};

#endif // ESP32WATERLEVELSENSOR_H
