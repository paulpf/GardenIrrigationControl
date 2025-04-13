#include "IrrigationZone.h"

IrrigationZone::IrrigationZone() 
{
  Trace::log("IrrigationZone constructor called.");
  _durationTime = DEFAULT_DURATION_TIME;
}

void IrrigationZone::setup(int hwBtnGpioChannel, int relayGpioChannel, String mqttTopicForZone) 
{
  // Setup code for the irrigation zone
  _hwBtnGpioChannel = hwBtnGpioChannel;
  _relayGpioChannel = relayGpioChannel;
  _mqttTopicForZone = mqttTopicForZone;
  _hwBtnState = false;
  _swBtnState = false;
  _synchronizedBtnNewState = false;
  setupHwButton(_hwBtnGpioChannel);
  setupRelay(_relayGpioChannel);
  Trace::log("IrrigationZone setup complete.");
}

void IrrigationZone::loadSettingsFromStorage(int zoneIndex) 
{
  _zoneIndex = zoneIndex;
  // Load duration time from storage
  _durationTime = StorageManager::getInstance().loadDurationTime(zoneIndex);
  Trace::log("Loaded settings for zone " + String(zoneIndex) + " with duration time: " + String(_durationTime));
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
  // We could save button state here, but it's usually transient
  // Uncomment below if you want to persist button states
  // StorageManager::getInstance().saveButtonState(_zoneIndex, newState);
}

void IrrigationZone::setupRelay(int relayGpioChannel) 
{
  pinMode(relayGpioChannel, OUTPUT);
  digitalWrite(relayGpioChannel, HIGH); // Set relay to HIGH (off) by default
}

void IrrigationZone::setRelayState(bool state) {
  _relaisState = state;
  // Save relay state to storage
  StorageManager::getInstance().saveRelayState(_zoneIndex, state);
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

int IrrigationZone::getRemainingTime() 
{
  if (!_timerIsActive) 
  {
    return 0; // Timer is not active, return 0
  }
  return _durationTime - (millis() - _startTime);
}

void IrrigationZone::startTimer() 
{
  _timerIsActive = true;
  _startTime = millis();
}

void IrrigationZone::resetTimer() 
{
  _timerIsActive = false;
}

void IrrigationZone::setDurationTime(int durationTime, int zoneIndex) {
  _durationTime = durationTime;
  _zoneIndex = zoneIndex;
  // Save duration time to permanent storage
  StorageManager::getInstance().saveDurationTime(zoneIndex, durationTime);
  Trace::log("Duration time for zone " + String(zoneIndex) + " set and saved to storage: " + String(durationTime));
}

void IrrigationZone::loop() 
{
  // Loop code for the irrigation zone
  Trace::log("IrrigationZone loop running.");

  if (getRelayState() == false && getBtnState() == true)
  {
    setRelayState(true);

    // Start timer
    startTimer();
  }
  // if btn1 is inactive, relais 1 should be inactive
  else if(getRelayState() == true && getBtnState() == false)
  {
    setRelayState(false);
    resetTimer();
  }

  if (getRelayState())
  {
    int remainingTime = getRemainingTime();
    Trace::log("Remaining time for relais1: " + String(remainingTime));
    if (remainingTime <= 0)
    {
      Trace::log("Relais1 timer expired");
      resetTimer();
      setRelayState(false);
      synchronizeButtonStates(false);
    }
  }

  switchRelay(getRelayState());
}