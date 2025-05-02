#include "StorageManager.h"

void StorageManager::begin() 
{
    _preferences.begin(_namespace, false); // false = read/write mode
    Trace::log(TraceLevel::DEBUG, "StorageManager initialized");
}

void StorageManager::saveDurationTime(int zoneIndex, int durationTime) 
{
    String key = getKey("duration", zoneIndex);
    _preferences.putInt(key.c_str(), durationTime);
    Trace::log(TraceLevel::DEBUG, "Saved duration time for zone " + String(zoneIndex) + ": " + String(durationTime));
}

int StorageManager::loadDurationTime(int zoneIndex) 
{
    String key = getKey("duration", zoneIndex);
    int durationTime = _preferences.getInt(key.c_str(), DEFAULT_DURATION_TIME);
    Trace::log(TraceLevel::DEBUG, "Loaded duration time for zone " + String(zoneIndex) + ": " + String(durationTime));
    return durationTime;
}

void StorageManager::saveButtonState(int zoneIndex, bool state) 
{
    String key = getKey("btn", zoneIndex);
    _preferences.putBool(key.c_str(), state);
}

bool StorageManager::loadButtonState(int zoneIndex) 
{
    String key = getKey("btn", zoneIndex);
    return _preferences.getBool(key.c_str(), false);
}

void StorageManager::saveRelayState(int zoneIndex, bool state) 
{
    String key = getKey("relay", zoneIndex);
    _preferences.putBool(key.c_str(), state);
}

bool StorageManager::loadRelayState(int zoneIndex) 
{
    String key = getKey("relay", zoneIndex);
    return _preferences.getBool(key.c_str(), false);
}

void StorageManager::clearAllSettings() 
{
    _preferences.clear();
    Trace::log(TraceLevel::DEBUG, "All stored settings cleared");
}

String StorageManager::getKey(const char* prefix, int zoneIndex) 
{
    return String(prefix) + String(zoneIndex);
}