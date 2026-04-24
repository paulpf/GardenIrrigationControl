#include "irrigation_zone.h"

bool IrrigationZone::_globalStartInhibit = false;

IrrigationZone::IrrigationZone()
    : _defaultDurationMs(DEFAULT_DURATION_TIME),
      _buttonDebounceMs(BUTTON_DEBOUNCE_TIME)
{
  Trace::log(TraceLevel::DEBUG, "IrrigationZone constructor called.");
  _durationTime = _defaultDurationMs;
}

void IrrigationZone::configure(const IrrigationConfig &config)
{
  _defaultDurationMs = config.defaultDurationMs;
  _buttonDebounceMs = config.buttonDebounceMs;
  _durationTime = _defaultDurationMs;
}

void IrrigationZone::setGlobalStartInhibit(bool inhibit)
{
  _globalStartInhibit = inhibit;
}

bool IrrigationZone::isGlobalStartInhibited()
{
  return _globalStartInhibit;
}

void IrrigationZone::setup(int hwBtnGpioChannel, int relayGpioChannel,
                           String mqttTopicForZone)
{
  // Setup code for the irrigation zone
  _hwBtnGpioChannel = hwBtnGpioChannel;
  _relayGpioChannel = relayGpioChannel;
  _mqttTopicForZone = mqttTopicForZone;
  resetTimer();                   // Reset timer to ensure clean state
  synchronizeButtonStates(false); // Initialize button states to false
  setupHwButton(_hwBtnGpioChannel);
  setupRelay(_relayGpioChannel);
  Trace::log(TraceLevel::DEBUG, "IrrigationZone setup complete.");
}

void IrrigationZone::loadSettingsFromStorage(int zoneIndex)
{
  _zoneIndex = zoneIndex;
  // Load duration time from storage, using configured default as fallback
  _durationTime = StorageManager::getInstance().loadDurationTime(zoneIndex, _defaultDurationMs);
  Trace::log(TraceLevel::DEBUG,
             "Loaded settings for zone " + String(zoneIndex) +
                 " with duration time: " + String(_durationTime));
}

void IrrigationZone::setupHwButton(int hwBtnGpioChannel)
{
  pinMode(hwBtnGpioChannel, INPUT_PULLDOWN);
  // Using ESP32's built-in support for std::function in attachInterruptArg
  attachInterruptArg(
      digitalPinToInterrupt(hwBtnGpioChannel), [](void *arg)
      { static_cast<IrrigationZone *>(arg)->onHwBtnPressed(); }, this, RISING);
}

void IRAM_ATTR IrrigationZone::onHwBtnPressed()
{
  unsigned long now = millis();
  if (now - _lastDebounceTime > _buttonDebounceMs)
  {
    _lastDebounceTime = now;
    _buttonEventPending = true;
    // Don't toggle state in ISR, just mark event for processing in main loop
  }
}

void IrrigationZone::synchronizeButtonStates(bool newState)
{
  Trace::log(TraceLevel::INFO, "Synchronizing of irrigation zone " +
                                   String(_zoneIndex + 1) +
                                   " button states to " + String(newState));
  _synchronizedBtnNewState = _swBtnState = _hwBtnState = newState;
  // We could save button state here, but it's usually transient
  // Uncomment below if you want to persist button states
  // StorageManager::getInstance().saveButtonState(_zoneIndex, newState);
}

void IrrigationZone::setupRelay(int relayGpioChannel)
{
  // Setup code for the relay
  // This is a workaround to avoid flickering of the relay when the ESP32 boots
  // up by setting the pin to LOW (active) and then to HIGH (off) after setup
  // This is a common practice to ensure the relay is in a known state
  // before the actual control logic starts
  pinMode(relayGpioChannel, INPUT);
  digitalWrite(relayGpioChannel,
               LOW); // Set relay to LOW (active) to avoid flickering
  pinMode(relayGpioChannel, OUTPUT);
  digitalWrite(relayGpioChannel, HIGH); // Set relay to HIGH (off) by default
}

void IrrigationZone::switchRelay(bool state)
{
  _relaisState = state;

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
  unsigned long elapsed = millis() - _startTime;
  return (elapsed < (unsigned long)_durationTime)
             ? (int)(_durationTime - elapsed)
             : 0;
}

/// @brief Method to get the remaining time as a formatted string in "MM:SS"
/// format.
/// @return A string representing the remaining time in minutes and seconds.
String IrrigationZone::getRemainingTimeAsString()
{
  int remainingTime = getRemainingTime();
  if (remainingTime <= 0)
  {
    return "00:00"; // Timer expired
  }

  int minutes = remainingTime / 60000; // Convert milliseconds to minutes
  int seconds = (remainingTime % 60000) / 1000; // Get remaining seconds

  // Format as MM:SS with leading zeros
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
  return String(buffer);
}

/// @brief Phase 3.5 optimized: Get remaining time into pre-allocated char
/// buffer
/// @param buffer Pointer to char array (must be at least 8 bytes)
/// @param bufsize Size of buffer
void IrrigationZone::getRemainingTimeAsString(char *buffer, size_t bufsize)
{
  if (!buffer || bufsize < 8)
  {
    return;
  }

  int remainingTime = getRemainingTime();
  if (remainingTime <= 0)
  {
    snprintf(buffer, bufsize, "00:00");
    return;
  }

  int minutes = remainingTime / 60000;
  int seconds = (remainingTime % 60000) / 1000;
  snprintf(buffer, bufsize, "%02d:%02d", minutes, seconds);
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

void IrrigationZone::setDurationTime(int durationTime, int zoneIndex)
{
  _durationTime = durationTime;
  _zoneIndex = zoneIndex;
  // Save duration time to permanent storage
  StorageManager::getInstance().saveDurationTime(zoneIndex, durationTime);
  Trace::log(TraceLevel::DEBUG,
             "Duration time for zone " + String(zoneIndex) +
                 " set and saved to storage: " + String(durationTime));
}

void IrrigationZone::loop()
{
  // Loop code for the irrigation zone
  Trace::log(TraceLevel::DEBUG,
             "IrrigationZone " + String(_zoneIndex) + " loop running.");

#ifdef ENABLE_LOOP_TIME_PLOTTING
  Trace::plotLoopTime("IrrigationZone", _zoneIndex, millis() - _loopStartTime);
  _loopStartTime = millis(); // Reset loop start time for next iteration
#endif

  if (_buttonEventPending)
  {
    _buttonEventPending = false;
    _hwBtnState = !_hwBtnState; // Toggle here instead of in ISR
    Trace::log(TraceLevel::INFO, "Hardware button of irrigation zone " +
                                     String(_zoneIndex + 1) + " pressed.");
    synchronizeButtonStates(_hwBtnState);
  }

  if (_globalStartInhibit)
  {
    if (getRelayState() == true || getBtnState() == true)
    {
      synchronizeButtonStates(false);
      switchRelay(false);
      resetTimer();
    }
    return;
  }

  if (getRelayState() == false && getBtnState() == true)
  {
    switchRelay(true);

    // Start timer
    startTimer();
  }
  // if btn1 is inactive, relais 1 should be inactive
  else if (getRelayState() == true && getBtnState() == false)
  {
    switchRelay(false);
    resetTimer();
  }

  if (getRelayState())
  {
    int remainingTime = getRemainingTime();
    Trace::log(TraceLevel::DEBUG, "Remaining time for relais " +
                                      String(_zoneIndex) + ": " +
                                      String(remainingTime));
    if (remainingTime <= 0)
    {
      Trace::log(TraceLevel::DEBUG,
                 "Relais timer " + String(_zoneIndex) + " expired");
      resetTimer();
      switchRelay(false);
      synchronizeButtonStates(false);
    }
  }
}