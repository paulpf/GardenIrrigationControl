#include "storage_manager.h"
#include "scheduled_irrigation.h"

void StorageManager::begin()
{
  _preferences.begin(_namespace, false); // false = read/write mode
  Trace::log(TraceLevel::DEBUG, "StorageManager initialized");
}

void StorageManager::saveDurationTime(int zoneIndex, int durationTime)
{
  String key = getKey("duration", zoneIndex);
  _preferences.putInt(key.c_str(), durationTime);
  Trace::log(TraceLevel::DEBUG, "Saved duration time for zone " +
                                    String(zoneIndex) + ": " +
                                    String(durationTime));
}

int StorageManager::loadDurationTime(int zoneIndex, int defaultDurationMs)
{
  String key = getKey("duration", zoneIndex);
  int durationTime = _preferences.getInt(key.c_str(), defaultDurationMs);
  if (durationTime <= 0)
  {
    Trace::log(TraceLevel::ERROR,
               "Invalid stored duration time for zone " + String(zoneIndex) +
                   ": " + String(durationTime) +
                   ". Restoring default: " + String(defaultDurationMs));
    saveDurationTime(zoneIndex, defaultDurationMs);
    return defaultDurationMs;
  }

  Trace::log(TraceLevel::DEBUG, "Loaded duration time for zone " +
                                    String(zoneIndex) + ": " +
                                    String(durationTime));
  return durationTime;
}

void StorageManager::saveAutomaticIrrigationEnabled(bool enabled)
{
  _preferences.putBool("autoEnabled", enabled);
}

bool StorageManager::loadAutomaticIrrigationEnabled(bool defaultValue)
{
  return _preferences.getBool("autoEnabled", defaultValue);
}

void StorageManager::saveLowWaterLockoutEnabled(bool enabled)
{
  _preferences.putBool("lwLockoutEn", enabled);
}

bool StorageManager::loadLowWaterLockoutEnabled(bool defaultValue)
{
  return _preferences.getBool("lwLockoutEn", defaultValue);
}

void StorageManager::saveSchedule(int zoneIndex, int scheduleIndex,
                                  const IrrigationSchedule &schedule)
{
  if (!isValidScheduleIndex(zoneIndex, scheduleIndex) ||
      !isValidSchedule(schedule))
  {
    Trace::log(TraceLevel::ERROR,
               "Cannot save invalid schedule for zone " + String(zoneIndex) +
                   ", schedule " + String(scheduleIndex));
    return;
  }

  _preferences.putBool(getScheduleKey("cfg", zoneIndex, scheduleIndex).c_str(),
                       true);
  _preferences.putBool(getScheduleKey("en", zoneIndex, scheduleIndex).c_str(),
                       schedule.enabled);
  _preferences.putInt(getScheduleKey("hr", zoneIndex, scheduleIndex).c_str(),
                      schedule.hour);
  _preferences.putInt(getScheduleKey("min", zoneIndex, scheduleIndex).c_str(),
                      schedule.minute);
  _preferences.putInt(getScheduleKey("dur", zoneIndex, scheduleIndex).c_str(),
                      schedule.durationMinutes);
  _preferences.putInt(getScheduleKey("dow", zoneIndex, scheduleIndex).c_str(),
                      (int)schedule.daysOfWeek);
}

bool StorageManager::loadSchedule(int zoneIndex, int scheduleIndex,
                                  IrrigationSchedule &schedule)
{
  if (!isValidScheduleIndex(zoneIndex, scheduleIndex) ||
      !isScheduleConfigured(zoneIndex, scheduleIndex))
  {
    return false;
  }

  IrrigationSchedule loaded;
  loaded.enabled = _preferences.getBool(
      getScheduleKey("en", zoneIndex, scheduleIndex).c_str(), false);
  loaded.hour =
      _preferences.getInt(getScheduleKey("hr", zoneIndex, scheduleIndex).c_str(),
                          loaded.hour);
  loaded.minute = _preferences.getInt(
      getScheduleKey("min", zoneIndex, scheduleIndex).c_str(), loaded.minute);
  loaded.durationMinutes = _preferences.getInt(
      getScheduleKey("dur", zoneIndex, scheduleIndex).c_str(),
      loaded.durationMinutes);
  loaded.daysOfWeek = (uint8_t)_preferences.getInt(
      getScheduleKey("dow", zoneIndex, scheduleIndex).c_str(),
      (int)loaded.daysOfWeek);

  if (!isValidSchedule(loaded))
  {
    Trace::log(TraceLevel::ERROR,
               "Invalid stored schedule for zone " + String(zoneIndex) +
                   ", schedule " + String(scheduleIndex) + ". Clearing slot.");
    clearSchedule(zoneIndex, scheduleIndex);
    return false;
  }

  schedule = loaded;
  return true;
}

void StorageManager::clearSchedule(int zoneIndex, int scheduleIndex)
{
  if (!isValidScheduleIndex(zoneIndex, scheduleIndex))
  {
    return;
  }

  _preferences.putBool(getScheduleKey("cfg", zoneIndex, scheduleIndex).c_str(),
                       false);
}

bool StorageManager::isScheduleConfigured(int zoneIndex, int scheduleIndex)
{
  if (!isValidScheduleIndex(zoneIndex, scheduleIndex))
  {
    return false;
  }

  return _preferences.getBool(
      getScheduleKey("cfg", zoneIndex, scheduleIndex).c_str(), false);
}

void StorageManager::clearAllSettings()
{
  _preferences.clear();
  Trace::log(TraceLevel::DEBUG, "All stored settings cleared");
}

String StorageManager::getKey(const char *prefix, int zoneIndex)
{
  return String(prefix) + String(zoneIndex);
}

String StorageManager::getScheduleKey(const char *field, int zoneIndex,
                                      int scheduleIndex)
{
  return "s" + String(zoneIndex) + "_" + String(scheduleIndex) + "_" +
         String(field);
}

bool StorageManager::isValidScheduleIndex(int zoneIndex,
                                          int scheduleIndex) const
{
  return zoneIndex >= 0 && zoneIndex < MAX_IRRIGATION_ZONES &&
         scheduleIndex >= 0 && scheduleIndex < MAX_SCHEDULES_PER_ZONE;
}

bool StorageManager::isValidSchedule(const IrrigationSchedule &schedule) const
{
  const int maxDurationMinutes = MAX_DURATION_TIME / 60000;
  return schedule.hour >= 0 && schedule.hour <= 23 && schedule.minute >= 0 &&
         schedule.minute <= 59 && schedule.durationMinutes > 0 &&
         schedule.durationMinutes <= maxDurationMinutes &&
         schedule.daysOfWeek >= 1 && schedule.daysOfWeek <= 127;
}

void StorageManager::factoryReset(int defaultDurationMs)
{
  Trace::log(TraceLevel::ERROR, "!!! FACTORY RESET INITIATED !!!");

  // Clear all existing settings
  clearAllSettings();

  // Restore all zones to default duration time
  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    saveDurationTime(i, defaultDurationMs);
  }
  saveAutomaticIrrigationEnabled(AUTOMATIC_IRRIGATION_ENABLED_DEFAULT);
  saveLowWaterLockoutEnabled(WATER_LEVEL_LOW_WATER_LOCKOUT_ENABLED);

  delay(100); // Ensure EEPROM writes complete
  Trace::log(TraceLevel::INFO, "Factory reset complete");
}
