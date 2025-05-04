#include "IrrigationZone.h"

IrrigationZone::IrrigationZone() 
{
  Trace::log(TraceLevel::DEBUG, "IrrigationZone constructor called.");
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
  Trace::log(TraceLevel::DEBUG, "IrrigationZone setup complete.");
}

void IrrigationZone::loadSettingsFromStorage(int zoneIndex) 
{
  _zoneIndex = zoneIndex;
  // Load duration time from storage
  _durationTime = StorageManager::getInstance().loadDurationTime(zoneIndex);
  Trace::log(TraceLevel::DEBUG, "Loaded settings for zone " + String(zoneIndex) + " with duration time: " + String(_durationTime));
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
  //Trace::log(TraceLevel::INFO, "Hardware button of irrigation zone " + String(_zoneIndex + 1) + " pressed.");
  unsigned long now = millis();
  if (now - _lastDebounceTime > BUTTON_DEBOUNCE_TIME) 
  {
    _lastDebounceTime = now;
    _buttonEventPending = true;
    // Don't toggle state in ISR, just mark event for processing in main loop
  }   
}

void IrrigationZone::synchronizeButtonStates(bool newState) 
{
  Trace::log(TraceLevel::INFO, "Synchronizing of irrigation zone " + String(_zoneIndex + 1) + " button states to " + String(newState));
  _synchronizedBtnNewState = _swBtnState = _hwBtnState = newState;
  // We could save button state here, but it's usually transient
  // Uncomment below if you want to persist button states
  // StorageManager::getInstance().saveButtonState(_zoneIndex, newState);
}

void IrrigationZone::setupRelay(int relayGpioChannel) 
{
  // Setup code for the relay
  // This is a workaround to avoid flickering of the relay when the ESP32 boots up
  // by setting the pin to LOW (active) and then to HIGH (off) after setup
  // This is a common practice to ensure the relay is in a known state
  // before the actual control logic starts
  pinMode(relayGpioChannel, INPUT);
  digitalWrite(relayGpioChannel, LOW); // Set relay to LOW (active) to avoid flickering  
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
    Trace::log(TraceLevel::DEBUG, "Switching relay ON");
    digitalWrite(_relayGpioChannel, LOW);
  }
  else
  {
    Trace::log(TraceLevel::DEBUG, "Switching relay OFF");
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
  Trace::log(TraceLevel::DEBUG, "Duration time for zone " + String(zoneIndex) + " set and saved to storage: " + String(durationTime));
}

void IrrigationZone::loop() 
{
  // Loop code for the irrigation zone
  Trace::log(TraceLevel::DEBUG, "IrrigationZone " + String(_zoneIndex) + " loop running.");

  if (_buttonEventPending) 
  {
    _buttonEventPending = false;
    _hwBtnState = !_hwBtnState; // Toggle here instead of in ISR
    Trace::log(TraceLevel::INFO, "Hardware button of irrigation zone " + String(_zoneIndex + 1) + " pressed.");
    synchronizeButtonStates(_hwBtnState);
  }

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
    Trace::log(TraceLevel::DEBUG, "Remaining time for relais " + String(_zoneIndex) + ": " + String(remainingTime));
    if (remainingTime <= 0)
    {
      Trace::log(TraceLevel::DEBUG, "Relais timer " + String(_zoneIndex) + " expired");
      resetTimer();
      setRelayState(false);
      synchronizeButtonStates(false);
    }
  }

  switchRelay(getRelayState());
}