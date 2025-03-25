#ifndef IRRIGATION_ZONE_H
#define IRRIGATION_ZONE_H

#include "GlobalDefines.h"
#include "HardwareButton.h"
#include "Relay.h"
#include "Timer.h"
#include "MqttManager.h"

class IrrigationZone {
public:
    IrrigationZone(int buttonPin, int relayPin, unsigned long debounceDelay, const String& clientName, MqttManager& mqttManager);

    void setup();
    void onTimerActivated();
    void onTimerDeactivated();
    void onTimerTick();
    void onHwButtonPressed();
    void handleSwBtnMessage(const String& message);
    void loop();

private:
    HardwareButton hwButton;
    Relay relay;
    Timer timer;

    bool synchronizedState;
    bool hwButtonState;
    bool swButtonState;

    String clientName;
    MqttManager& mqttManager;

    void updateTimerState();
};

#endif // IRRIGATION_ZONE_H