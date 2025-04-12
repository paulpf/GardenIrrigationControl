#ifndef IRRIGATION_ZONE_H
#define IRRIGATION_ZONE_H

#include "globalDefines.h"

class IrrigationZone {
public:
    IrrigationZone();

    void setup(int hwBtnGpioChannel, String mqttTopic);
    void loop();
    String getMqttTopicForSwButton();
    void synchronizeButtonStates(bool newState);
    bool getBtnState() { return _hwBtnState; } // Get the hardware button state

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
};

#endif // IRRIGATION_ZONE_H