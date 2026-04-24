#ifndef HARDWARECONFIG_H
#define HARDWARECONFIG_H

#include "config.h"

struct HardwareConfig
{
  // Water level sensor ADC pin and conversion range
  int waterLevelSensorPin = WATER_LEVEL_SENSOR_PIN;
  int waterLevelAdcMin = WATER_LEVEL_ADC_MIN;
  int waterLevelAdcMax = WATER_LEVEL_ADC_MAX;
  
  // Button/relay pin pairs for each zone (8 regular zones + 1 drainage zone)
  struct ZonePins {
    int buttonPin;
    int relayPin;
  };
  
  ZonePins zones[9] = {
    {ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN},
    {ZONE2_BUTTON_PIN, ZONE2_RELAY_PIN},
    {ZONE3_BUTTON_PIN, ZONE3_RELAY_PIN},
    {ZONE4_BUTTON_PIN, ZONE4_RELAY_PIN},
    {ZONE5_BUTTON_PIN, ZONE5_RELAY_PIN},
    {ZONE6_BUTTON_PIN, ZONE6_RELAY_PIN},
    {ZONE7_BUTTON_PIN, ZONE7_RELAY_PIN},
    {ZONE8_BUTTON_PIN, ZONE8_RELAY_PIN},
    {ZONE9_BUTTON_PIN, ZONE9_RELAY_PIN},  // Drainage
  };
};

#endif // HARDWARECONFIG_H
