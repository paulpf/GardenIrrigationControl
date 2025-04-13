#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "globaldefines.h"
#include <Preferences.h>

class StorageManager {
public:
    static StorageManager& getInstance() 
    {
        static StorageManager instance;
        return instance;
    }
    
    void begin();
    
    // Methods for irrigation zone settings
    void saveDurationTime(int zoneIndex, int durationTime);
    int loadDurationTime(int zoneIndex);
    
    // Methods for button states - may not be needed as they're typically volatile
    void saveButtonState(int zoneIndex, bool state);
    bool loadButtonState(int zoneIndex);
    
    // Relay state is also mostly volatile, but might be useful for recovering state after power loss
    void saveRelayState(int zoneIndex, bool state);
    bool loadRelayState(int zoneIndex);
    
    // Method to clear all saved settings (useful for testing or factory reset)
    void clearAllSettings();

private:
    StorageManager() {} // Private constructor for singleton
    Preferences _preferences;
    const char* _namespace = "garden_ctrl";
    
    // Key generation helper
    String getKey(const char* prefix, int zoneIndex);
};

#endif // STORAGE_MANAGER_H