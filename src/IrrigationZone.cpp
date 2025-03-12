#include "IrrigationZone.h"
#include <Arduino.h>
#include "Trace.h"

IrrigationZone::IrrigationZone()
{
}

IrrigationZone::~IrrigationZone()
{
}

void IrrigationZone::setup()
{
}

bool IrrigationZone::getRelaisState(int relayId)
{
  // Here is to implement the logic to get the state of the relay
  Trace::log("SwBtnState: " + String(_swBtnState) + " | HwBtnState: " + String(_hwBtnState));
  if (_swBtnState || _hwBtnState)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void IrrigationZone::setSwBtnState(bool btnState)
{
  _swBtnState = btnState;
}

void IrrigationZone::setHwBtnState(bool btnState)
{
  _hwBtnState = btnState;
}

void IrrigationZone::setDuration(int duration)
{
  _duration = duration;
}