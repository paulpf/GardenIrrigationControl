#include "esp32waterlevelsensor.h"

#include "config.h"
#include "global_defines.h"

void Esp32WaterLevelSensor::setup()
{
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);
  analogSetPinAttenuation(WATER_LEVEL_SENSOR_PIN, ADC_11db);
}

int Esp32WaterLevelSensor::readRaw()
{
  return analogRead(WATER_LEVEL_SENSOR_PIN);
}
