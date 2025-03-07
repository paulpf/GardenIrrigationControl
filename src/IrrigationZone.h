// IrrigationZone.h
#ifndef IRRIGATIONZONE_H
#define IRRIGATIONZONE_H

#include <Arduino.h>
#include "FunctionalInterrupt.h"

class IrrigationZone 
{
public:
    IrrigationZone();
    void setup(int hwBtnPin, int relayPin, unsigned long relayDuration);
    void handleBtn1Pressed();
    void switchRelayON();
    void switchRelayOFF();
    void update();

private:
    int _hwBtnPin;
    int _relayPin;
    unsigned long _relayOnDuration;
    unsigned long _lastDebounceTime;
    unsigned long _relayOnStartTime;
    const int _debounceDelay = 500; // debounce time in milliseconds
    volatile bool _btnPressed;
    bool _relayState;


    void printToConsole(String message);
};


#endif