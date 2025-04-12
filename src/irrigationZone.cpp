#include "IrrigationZone.h"

IrrigationZone::IrrigationZone() 
{
  Trace::log("IrrigationZone constructor called.");
  // Constructor implementation (if needed)
}

void IrrigationZone::setup(int hwBtnGpioChannel, int relayGpioChannel, String mqttTopicForZone) 
{
  // Setup code for the irrigation zone
  Trace::log("IrrigationZone setup complete.");
  _hwBtnGpioChannel = hwBtnGpioChannel;
  _relayGpioChannel = relayGpioChannel;
  _mqttTopicForSwButton = mqttTopicForZone + "/swBtn";
  _hwBtnState = false;
  _swBtnState = false;
  _synchronizedBtnNewState = false;
  setupHwButton(_hwBtnGpioChannel);
  setupRelay(_relayGpioChannel);
}

void IrrigationZone::setupHwButton(int hwBtnGpioChannel)
{
  pinMode(hwBtnGpioChannel, INPUT_PULLDOWN);
  // Using ESP32's built-in support for std::function in attachInterruptArg
  attachInterruptArg(
    digitalPinToInterrupt(hwBtnGpioChannel),
    [](void* arg) { static_cast<IrrigationZone*>(arg)->onHwBtnPressed(); },
    this,
    RISING
  );
}

void IRAM_ATTR IrrigationZone::onHwBtnPressed() 
{
  Trace::log("Hardware button pressed.");
  unsigned long now = millis();
  if (now - _lastDebounceTime > _debounceDelay) 
  {
    _lastDebounceTime = now;
    _hwBtnState = !_hwBtnState;
    // synchronize the new state with the software state
    synchronizeButtonStates(_hwBtnState);
  }  
}

void IrrigationZone::synchronizeButtonStates(bool newState) 
{
  _synchronizedBtnNewState = _swBtnState = _hwBtnState = newState;
}

void IrrigationZone::loop() 
{
  // Loop code for the irrigation zone
  Trace::log("IrrigationZone loop running.");
}

String IrrigationZone::getMqttTopicForSwButton() 
{
  return _mqttTopicForSwButton;
}

void IrrigationZone::setupRelay(int relayGpioChannel) 
{
  pinMode(relayGpioChannel, OUTPUT);
  digitalWrite(relayGpioChannel, HIGH); // Set relay to HIGH (off) by default
}

void IrrigationZone::switchRelay(bool state)
{
  if (state)
  {
    Trace::log("Switching relay ON");
    digitalWrite(_relayGpioChannel, LOW);
  }
  else
  {
    Trace::log("Switching relay OFF");
    digitalWrite(_relayGpioChannel, HIGH);
  }
}