#include "waterlevelmanager.h"

#include "irrigation_zone.h"

WaterLevelManager::WaterLevelManager(IMessagePublisher &messagePublisher)
    : _messagePublisher(messagePublisher)
{
}

void WaterLevelManager::setup(const char *clientName)
{
  initTopics(clientName);
  initSensor();
}

void WaterLevelManager::loop(unsigned long currentMillis)
{
  if (!shouldRead(currentMillis))
  {
    return;
  }

  updateMetrics();
  logSnapshot();
  const TransitionEvents events = updateStateTransitions();
  applyTransitionEffects(events);
  publishData();
}

void WaterLevelManager::initTopics(const char *clientName)
{
  _topics.percent = String(clientName) + "/waterlevel/percent";
  _topics.liters = String(clientName) + "/waterlevel/liters";
  _topics.raw = String(clientName) + "/waterlevel/raw";
  _topics.status = String(clientName) + "/waterlevel/status";
  _topics.lockout = String(clientName) + "/waterlevel/lockout";
  _topics.criticalHighAlarm =
      String(clientName) + "/waterlevel/critical_high_alarm";
  _topics.overflow = String(clientName) + "/waterlevel/overflow";
  _topics.safetyLock = String(clientName) + "/waterlevel/safety_lock";
  _topics.overflowLiters = String(clientName) + "/waterlevel/overflow_liters";
  _topics.litersToOverflow =
      String(clientName) + "/waterlevel/liters_to_overflow";
}

void WaterLevelManager::initSensor()
{
  pinMode(WATER_LEVEL_SENSOR_PIN, INPUT);
  analogSetPinAttenuation(WATER_LEVEL_SENSOR_PIN, ADC_11db);
}

bool WaterLevelManager::shouldRead(unsigned long currentMillis)
{
  if ((unsigned long)(currentMillis - _previousRead) <
      WATER_LEVEL_READ_INTERVAL)
  {
    return false;
  }

  _previousRead = currentMillis;
  return true;
}

void WaterLevelManager::updateMetrics()
{
  _state.rawValue = analogRead(WATER_LEVEL_SENSOR_PIN);

  if (WATER_LEVEL_ADC_MAX > WATER_LEVEL_ADC_MIN)
  {
    _state.percent = ((float)(_state.rawValue - WATER_LEVEL_ADC_MIN) * 100.0f) /
                     (float)(WATER_LEVEL_ADC_MAX - WATER_LEVEL_ADC_MIN);
  }
  else
  {
    _state.percent = 0.0f;
  }

  if (_state.percent < 0.0f)
  {
    _state.percent = 0.0f;
  }

  _state.liters = (_state.percent / 100.0f) * CISTERN_CAPACITY_LITERS;

  const float overflowThresholdLiters =
      (WATER_LEVEL_OVERFLOW_PERCENT / 100.0f) * CISTERN_CAPACITY_LITERS;
  if (_state.liters > overflowThresholdLiters)
  {
    _state.overflowLiters = _state.liters - overflowThresholdLiters;
  }
  else
  {
    _state.overflowLiters = 0.0f;
  }

  if (_state.liters < overflowThresholdLiters)
  {
    _state.litersToOverflow = overflowThresholdLiters - _state.liters;
  }
  else
  {
    _state.litersToOverflow = 0.0f;
  }
}

void WaterLevelManager::logSnapshot() const
{
  Trace::log(TraceLevel::DEBUG, "Water level raw=" + String(_state.rawValue) +
                                    " percent=" + String(_state.percent, 1) +
                                    " liters=" + String(_state.liters, 1));
}

WaterLevelManager::TransitionEvents WaterLevelManager::updateStateTransitions()
{
  TransitionEvents events;

  const bool shouldEnableLowWaterLockout =
      _state.percent < WATER_LEVEL_CRITICAL_PERCENT;
  const bool shouldReleaseLowWaterLockout =
      _state.percent >= WATER_LEVEL_LOCKOUT_RELEASE_PERCENT;

  if (!_state.lowWaterLockoutActive && shouldEnableLowWaterLockout)
  {
    _state.lowWaterLockoutActive = true;
    events.lowWaterLockoutChanged = true;
  }
  else if (_state.lowWaterLockoutActive && shouldReleaseLowWaterLockout)
  {
    _state.lowWaterLockoutActive = false;
    events.lowWaterLockoutChanged = true;
  }

  const bool shouldEnableCriticalOverflowAlarm =
      _state.overflowLiters >= WATER_LEVEL_CRITICAL_OVERFLOW_BUFFER_LITERS;
  const bool shouldReleaseCriticalOverflowAlarm =
      _state.overflowLiters <= WATER_LEVEL_CRITICAL_OVERFLOW_RELEASE_LITERS;

  if (!_state.criticalOverflowAlarmActive && shouldEnableCriticalOverflowAlarm)
  {
    _state.criticalOverflowAlarmActive = true;
    events.criticalOverflowAlarmChanged = true;
  }
  else if (_state.criticalOverflowAlarmActive &&
           shouldReleaseCriticalOverflowAlarm)
  {
    _state.criticalOverflowAlarmActive = false;
    events.criticalOverflowAlarmChanged = true;
  }

  if (!_state.overflowActive && _state.percent > WATER_LEVEL_OVERFLOW_PERCENT)
  {
    _state.overflowActive = true;
    events.overflowChanged = true;
  }
  else if (_state.overflowActive &&
           _state.percent <= WATER_LEVEL_OVERFLOW_CLEAR_PERCENT)
  {
    _state.overflowActive = false;
    events.overflowChanged = true;
  }

  const bool shouldSafetyLock =
      _state.lowWaterLockoutActive || _state.criticalOverflowAlarmActive;
  if (_state.safetyLockActive != shouldSafetyLock)
  {
    _state.safetyLockActive = shouldSafetyLock;
    events.safetyLockChanged = true;
  }

  return events;
}

void WaterLevelManager::applyTransitionEffects(const TransitionEvents &events)
{
  if (events.lowWaterLockoutChanged)
  {
    if (_state.lowWaterLockoutActive)
    {
      Trace::log(TraceLevel::ERROR, "Low water lockout ACTIVATED: " +
                                        String(_state.percent, 1) + "%");
    }
    else
    {
      Trace::log(TraceLevel::INFO, "Low water lockout DEACTIVATED: " +
                                       String(_state.percent, 1) + "%");
    }

    publishStateChange(_topics.lockout.c_str(), _state.lowWaterLockoutActive);
  }

  if (events.criticalOverflowAlarmChanged)
  {
    if (_state.criticalOverflowAlarmActive)
    {
      Trace::log(TraceLevel::ERROR, "Critical overflow alarm ACTIVATED: " +
                                        String(_state.overflowLiters, 1) + "L");
    }
    else
    {
      Trace::log(TraceLevel::INFO, "Critical overflow alarm DEACTIVATED: " +
                                       String(_state.overflowLiters, 1) + "L");
    }

    publishStateChange(_topics.criticalHighAlarm.c_str(),
                       _state.criticalOverflowAlarmActive);
  }

  if (events.overflowChanged)
  {
    if (_state.overflowActive)
    {
      Trace::log(TraceLevel::ERROR, "Cistern overflow detected: " +
                                        String(_state.percent, 1) + "%");
    }
    else
    {
      Trace::log(TraceLevel::INFO, "Cistern overflow cleared: " +
                                       String(_state.percent, 1) + "%");
    }

    publishStateChange(_topics.overflow.c_str(), _state.overflowActive);
  }

  if (events.safetyLockChanged)
  {
    IrrigationZone::setGlobalStartInhibit(_state.safetyLockActive);

    if (_state.safetyLockActive)
    {
      Trace::log(TraceLevel::ERROR, "Water level safety lock ACTIVATED");
    }
    else
    {
      Trace::log(TraceLevel::INFO, "Water level safety lock RELEASED");
    }

    publishStateChange(_topics.safetyLock.c_str(), _state.safetyLockActive);
  }
}

void WaterLevelManager::publishStateChange(const char *topic, bool active)
{
  if (_messagePublisher.isConnected())
  {
    _messagePublisher.publish(topic, active ? "true" : "false");
  }
}

void WaterLevelManager::publishData()
{
  if (!_messagePublisher.isConnected())
  {
    return;
  }

  _messagePublisher.publish(_topics.percent.c_str(),
                            String(_state.percent, 1).c_str());
  _messagePublisher.publish(_topics.liters.c_str(),
                            String(_state.liters, 1).c_str());
  _messagePublisher.publish(_topics.raw.c_str(),
                            String(_state.rawValue).c_str());
  _messagePublisher.publish(_topics.criticalHighAlarm.c_str(),
                            _state.criticalOverflowAlarmActive ? "true"
                                                               : "false");
  _messagePublisher.publish(_topics.safetyLock.c_str(),
                            _state.safetyLockActive ? "true" : "false");
  _messagePublisher.publish(_topics.overflowLiters.c_str(),
                            String(_state.overflowLiters, 1).c_str());
  _messagePublisher.publish(_topics.litersToOverflow.c_str(),
                            String(_state.litersToOverflow, 1).c_str());
  _messagePublisher.publish(_topics.status.c_str(), "online");
}