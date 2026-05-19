#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "global_defines.h"
#include <Preferences.h>

struct IrrigationSchedule;

class StorageManager
{
public:
  static StorageManager &getInstance()
  {
    static StorageManager instance;
    return instance;
  }

  void begin();

  // Methods for irrigation zone settings
  void saveDurationTime(int zoneIndex, int durationTime);
  int loadDurationTime(int zoneIndex, int defaultDurationMs = DEFAULT_DURATION_TIME);

  // Global runtime settings
  void saveAutomaticIrrigationEnabled(bool enabled);
  bool loadAutomaticIrrigationEnabled(bool defaultValue = false);
  void saveLowWaterLockoutEnabled(bool enabled);
  bool loadLowWaterLockoutEnabled(bool defaultValue = WATER_LEVEL_LOW_WATER_LOCKOUT_ENABLED);

  // Methods for scheduled irrigation settings
  void saveSchedule(int zoneIndex, int scheduleIndex,
                    const IrrigationSchedule &schedule);
  bool loadSchedule(int zoneIndex, int scheduleIndex,
                    IrrigationSchedule &schedule);
  void clearSchedule(int zoneIndex, int scheduleIndex);
  bool isScheduleConfigured(int zoneIndex, int scheduleIndex);

  // Method to clear all saved settings (useful for testing or factory reset)
  void clearAllSettings();

  // Factory reset - restore all zones to default configuration
  void factoryReset(int defaultDurationMs = DEFAULT_DURATION_TIME);

private:
  StorageManager()
  {
  } // Private constructor for singleton
  Preferences _preferences;
  const char *_namespace = "garden_ctrl";

  // Key generation helper
  String getKey(const char *prefix, int zoneIndex);
  String getScheduleKey(const char *field, int zoneIndex, int scheduleIndex);
  bool isValidScheduleIndex(int zoneIndex, int scheduleIndex) const;
  bool isValidSchedule(const IrrigationSchedule &schedule) const;
};

#endif // STORAGE_MANAGER_H
