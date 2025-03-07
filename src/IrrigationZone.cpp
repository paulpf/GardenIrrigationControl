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
    attachInterrupt(digitalPinToInterrupt(_hwBtnPin), std::bind(&IrrigationZone::OnHwBtnPressed, this), RISING);
}

void IrrigationZone::trace(String message)
{
    #ifdef TRACE
    String timeStamp = String(millis());
    Serial.println(timeStamp + " | " + message);
    #endif
}

void IRAM_ATTR IrrigationZone::OnHwBtnPressed() 
{
    trace("Interrupt triggered");
    unsigned long now = millis();
    if (now - _lastDebounceTime > _debounceDelay) 
    {
        _lastDebounceTime = now;
        _btnPressed = true;
        trace("Button pressed");
    }
}

void IrrigationZone::switchRelayON()
{
    _relayState = HIGH;
    digitalWrite(_relayPin, LOW);
    trace("Relay switched on");
}

void IrrigationZone::switchRelayOFF()
{
    _relayState = LOW;
    digitalWrite(_relayPin, HIGH);
    trace("Relay switched off");
}

void IrrigationZone::update()
{
    if (_btnPressed) 
    {
        _btnPressed = false;
        trace("Button pressed quit!");

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