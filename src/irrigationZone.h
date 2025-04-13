#ifndef IRRIGATION_ZONE_H
#define IRRIGATION_ZONE_H

#include "globalDefines.h"
#include "StorageManager.h"

class IrrigationZone {
public:
    IrrigationZone();

    void setup(int hwBtnGpioChannel, int relayGpioChannel, String mqttTopicForZone);
    void loadSettingsFromStorage(int zoneIndex);
    void loop();

    // ================ Mqtt topics ================
    String getMqttTopicForZone() { return _mqttTopicForZone; } // Get the MQTT topic for the zone
    String getMqttTopicForSwButton() { return _mqttTopicForZone + "/swBtn"; } // Get the MQTT topic for the software button
    String getMqttTopicForRelay() { return _mqttTopicForZone + "/relayState"; } // Get the MQTT topic for the relay state
    String getMqttTopicForRemainingTime() { return _mqttTopicForZone + "/remainingTime"; } // Get the MQTT topic for the remaining time
    String getMqttTopicForDurationTime() { return _mqttTopicForZone + "/durationTime"; } // Get the MQTT topic for the duration time
    
    void synchronizeButtonStates(bool newState);
    bool getBtnState() { return _hwBtnState; } // Get the hardware button state
    void switchRelay(bool state);
    bool getRelayState() { return _relaisState; } // Get the relay state
    void setRelayState(bool state); // Set the relay state
    void startTimer(); // Set the relay start time
    unsigned long getStartTime() { return _startTime; } // Get the relay start time
    void setDurationTime(int durationTime, int zoneIndex); // Set the relay duration time
    int getDurationTime() { return _durationTime; } // Get the relay duration time
    int getRemainingTime(); // Get the remaining time for the relay
    void resetTimer();

private:
    // ================ Mqtt topics ================
    String _mqttTopicForZone;
    String _mqttTopicForSwButton;

    // ================ Hardware button ================
    int _hwBtnGpioChannel;
    bool _hwBtnState = false;
    unsigned long _lastDebounceTime;
    const int _debounceDelay = 500; // debounce time in milliseconds
    bool _synchronizedBtnNewState = false;
    //bool _synconizedBtnOldState = false;

    void setupHwButton(int hwBtnGpioChannel);
    void IRAM_ATTR onHwBtnPressed();
    
    // ================ Software button (via MQTT) ================
    bool _swBtnState = false;
    bool _swBtnStateOld = false;

    // ================ Relay ================
    int _relayGpioChannel;
    bool _relaisState = false;
    void setupRelay(int relayGpioChannel);

    // ================ timer ================
    bool _timerIsActive = false;
    unsigned long _startTime;
    int _durationTime;
    
    // ================ Storage ================
    int _zoneIndex; // To track which zone index this is for storage
};

#endif // IRRIGATION_ZONE_H