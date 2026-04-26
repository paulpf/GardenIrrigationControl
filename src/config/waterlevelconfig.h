#ifndef WATERLEVELCONFIG_H
#define WATERLEVELCONFIG_H

#include "config.h"

struct WaterLevelConfig
{
  bool lowWaterLockoutEnabled = WATER_LEVEL_LOW_WATER_LOCKOUT_ENABLED;
  int adcMin = WATER_LEVEL_ADC_MIN;
  int adcMax = WATER_LEVEL_ADC_MAX;
  float capacityLiters = CISTERN_CAPACITY_LITERS;
  float criticalPercent = WATER_LEVEL_CRITICAL_PERCENT;
  float lockoutReleasePercent = WATER_LEVEL_LOCKOUT_RELEASE_PERCENT;
  float overflowPercent = WATER_LEVEL_OVERFLOW_PERCENT;
  float overflowClearPercent = WATER_LEVEL_OVERFLOW_CLEAR_PERCENT;
  float criticalOverflowBufferLiters =
      WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;
  float criticalOverflowReleaseLiters =
      WATER_LEVEL_CRITICAL_OVERFLOW_RELEASE_LITERS;
  unsigned long readIntervalMs = WATER_LEVEL_READ_INTERVAL;
};

#endif // WATERLEVELCONFIG_H
