#include "IrrigationZone.h"
#include <Arduino.h>
#include "FunctionalInterrupt.h"

IrrigationZone::IrrigationZone()
{

}

void IrrigationZone::setup(int hwBtnPin, int relayPin, unsigned long relayDuration)
{
    _hwBtnPin = hwBtnPin;
    _relayPin = relayPin;
    _relayOnDuration = relayDuration;
    pinMode(_hwBtnPin, INPUT_PULLDOWN);
    pinMode(_relayPin, OUTPUT);
    switchRelayOFF();
    attachInterrupt(digitalPinToInterrupt(_hwBtnPin), std::bind(&IrrigationZone::handleBtn1Pressed, this), RISING);
}

void IrrigationZone::printToConsole(String message)
{
    String timeStamp = String(millis());
    Serial.println(timeStamp + " | " + message);
}

void IRAM_ATTR IrrigationZone::handleBtn1Pressed() 
{
    printToConsole("Interrupt triggered");
    unsigned long now = millis();
    if (now - _lastDebounceTime > _debounceDelay) 
    {
        _lastDebounceTime = now;
        _btnPressed = true;
        printToConsole("Button pressed");
    }
}

void IrrigationZone::switchRelayON()
{
    _relayState = HIGH;
    digitalWrite(_relayPin, LOW);
    printToConsole("Relay switched on");
}

void IrrigationZone::switchRelayOFF()
{
    _relayState = LOW;
    digitalWrite(_relayPin, HIGH);
    printToConsole("Relay switched off");
}

void IrrigationZone::update()
{
    if (_btnPressed) 
    {
        _btnPressed = false;
        printToConsole("Button pressed quit!");

        if (_relayState == HIGH) 
        {
            switchRelayOFF();
        }
        else
        {
            switchRelayON();
            _relayOnStartTime = millis();
        }    
    }
  
    if (_relayState == HIGH && (millis() - _relayOnStartTime >= _relayOnDuration)) 
    {
        switchRelayOFF();
    }
}