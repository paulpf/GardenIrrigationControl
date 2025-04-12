#ifndef IRRIGATION_ZONE_H
#define IRRIGATION_ZONE_H

#include "globalDefines.h"

class IrrigationZone {
public:
    IrrigationZone();

    void setup(int hwBtnGpioChannel, int relayGpioChannel, String mqttTopic);
    void loop();
    String getMqttTopicForSwButton();
    void synchronizeButtonStates(bool newState);
    bool getBtnState() { return _hwBtnState; } // Get the hardware button state
    void switchRelay(bool state);
    bool getRelayState() { return _relaisState; } // Get the relay state
    void setRelayState(bool state) { _relaisState = state; } // Set the relay state

private:
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
};

#endif // IRRIGATION_ZONE_H