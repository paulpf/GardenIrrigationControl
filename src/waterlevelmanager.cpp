#include "waterlevelmanager.h"

#include "irrigation_zone.h"

WaterLevelManager::WaterLevelManager(IMessagePublisher &messagePublisher,
                   IWaterLevelSensorReader &sensor,
                   const WaterLevelConfig &config)
  : _messagePublisher(messagePublisher), _sensor(sensor), _config(config)
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
  _sensor.setup();
}

bool WaterLevelManager::shouldRead(unsigned long currentMillis)
{
  if ((unsigned long)(currentMillis - _previousRead) <
      _config.readIntervalMs)
  {
    return false;
  }

  _previousRead = currentMillis;
  return true;
}

void WaterLevelManager::updateMetrics()
{
  _state.rawValue = _sensor.readRaw();

  if (_config.adcMax > _config.adcMin)
  {
    _state.percent = ((float)(_state.rawValue - _config.adcMin) * 100.0f) /
                     (float)(_config.adcMax - _config.adcMin);
  }
  else
  {
    _state.percent = 0.0f;
  }

  if (_state.percent < 0.0f)
  {
    _state.percent = 0.0f;
  }

  _state.liters = (_state.percent / 100.0f) * _config.capacityLiters;

  const float overflowThresholdLiters =
      (_config.overflowPercent / 100.0f) * _config.capacityLiters;
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
      _state.percent < _config.criticalPercent;
  const bool shouldReleaseLowWaterLockout =
      _state.percent >= _config.lockoutReleasePercent;

  if (!_state.lowWaterLockoutActive && shouldEnableLowWaterLockout)
  {
    _state.lowWaterLockoutActive = true;
    events.add(TransitionEvents::Type::LowWaterLockout,
               _state.lowWaterLockoutActive);
  }
  else if (_state.lowWaterLockoutActive && shouldReleaseLowWaterLockout)
  {
    _state.lowWaterLockoutActive = false;
    events.add(TransitionEvents::Type::LowWaterLockout,
               _state.lowWaterLockoutActive);
  }

  const bool shouldEnableCriticalOverflowAlarm =
      _state.overflowLiters >= _config.criticalOverflowBufferLiters;
  const bool shouldReleaseCriticalOverflowAlarm =
      _state.overflowLiters <= _config.criticalOverflowReleaseLiters;

  if (!_state.criticalOverflowAlarmActive && shouldEnableCriticalOverflowAlarm)
  {
    _state.criticalOverflowAlarmActive = true;
    events.add(TransitionEvents::Type::CriticalOverflowAlarm,
               _state.criticalOverflowAlarmActive);
  }
  else if (_state.criticalOverflowAlarmActive &&
           shouldReleaseCriticalOverflowAlarm)
  {
    _state.criticalOverflowAlarmActive = false;
    events.add(TransitionEvents::Type::CriticalOverflowAlarm,
               _state.criticalOverflowAlarmActive);
  }

  if (!_state.overflowActive && _state.percent > _config.overflowPercent)
  {
    _state.overflowActive = true;
    events.add(TransitionEvents::Type::Overflow, _state.overflowActive);
  }
  else if (_state.overflowActive &&
           _state.percent <= _config.overflowClearPercent)
  {
    _state.overflowActive = false;
    events.add(TransitionEvents::Type::Overflow, _state.overflowActive);
  }

  const bool shouldSafetyLock =
      _state.lowWaterLockoutActive || _state.criticalOverflowAlarmActive;
  if (_state.safetyLockActive != shouldSafetyLock)
  {
    _state.safetyLockActive = shouldSafetyLock;
    events.add(TransitionEvents::Type::SafetyLock, _state.safetyLockActive);
  }

  return events;
}

void WaterLevelManager::applyTransitionEffects(const TransitionEvents &events)
{
  for (int i = 0; i < events.count; i++)
  {
    const TransitionEvents::Event &event = events.events[i];

    if (event.type == TransitionEvents::Type::LowWaterLockout)
    {
      if (event.active)
      {
        Trace::log(TraceLevel::ERROR, "Low water lockout ACTIVATED: " +
                                          String(_state.percent, 1) + "%");
      }
      else
      {
        Trace::log(TraceLevel::INFO, "Low water lockout DEACTIVATED: " +
                                         String(_state.percent, 1) + "%");
      }

      publishStateChange(_topics.lockout.c_str(), event.active);
    }
    else if (event.type == TransitionEvents::Type::CriticalOverflowAlarm)
    {
      if (event.active)
      {
        Trace::log(TraceLevel::ERROR, "Critical overflow alarm ACTIVATED: " +
                                          String(_state.overflowLiters, 1) +
                                          "L");
      }
      else
      {
        Trace::log(TraceLevel::INFO, "Critical overflow alarm DEACTIVATED: " +
                                         String(_state.overflowLiters, 1) +
                                         "L");
      }

      publishStateChange(_topics.criticalHighAlarm.c_str(), event.active);
    }
    else if (event.type == TransitionEvents::Type::Overflow)
    {
      if (event.active)
      {
        Trace::log(TraceLevel::ERROR, "Cistern overflow detected: " +
                                          String(_state.percent, 1) + "%");
      }
      else
      {
        Trace::log(TraceLevel::INFO, "Cistern overflow cleared: " +
                                         String(_state.percent, 1) + "%");
      }

      publishStateChange(_topics.overflow.c_str(), event.active);
    }
    else if (event.type == TransitionEvents::Type::SafetyLock)
    {
      IrrigationZone::setGlobalStartInhibit(event.active);

      if (event.active)
      {
        Trace::log(TraceLevel::ERROR, "Water level safety lock ACTIVATED");
      }
      else
      {
        Trace::log(TraceLevel::INFO, "Water level safety lock RELEASED");
      }

      publishStateChange(_topics.safetyLock.c_str(), event.active);
    }
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
  _messagePublisher.publish(_topics.overflowLiters.c_str(),
                            String(_state.overflowLiters, 1).c_str());
  _messagePublisher.publish(_topics.litersToOverflow.c_str(),
                            String(_state.litersToOverflow, 1).c_str());
  _messagePublisher.publish(_topics.status.c_str(), "online");
}