#include "scheduled_irrigation.h"
#include "trace.h"

ScheduledIrrigation::ScheduledIrrigation()
    : _zoneIndex(-1), _wallClock(nullptr), _scheduleCount(0),
      _lastTriggerKey(0), _hasLastTriggerKey(false)
{
}

void ScheduledIrrigation::setup(int zoneIndex)
{
  setup(zoneIndex, nullptr);
}

void ScheduledIrrigation::setup(int zoneIndex, IWallClock *wallClock)
{
  _zoneIndex = zoneIndex;
  _wallClock = wallClock;
  _scheduleCount = 0;
  _lastTriggerKey = 0;
  _hasLastTriggerKey = false;
  Trace::log(TraceLevel::INFO,
             "ScheduledIrrigation initialized for zone " + String(zoneIndex));
}

void ScheduledIrrigation::loop(unsigned long currentTime)
{
  IrrigationSchedule dueSchedule;
  if (!consumeDueSchedule(currentTime, dueSchedule))
  {
    return;
  }

  Trace::log(TraceLevel::INFO,
             "Schedule triggered for zone " + String(_zoneIndex) +
                 " - running for " + String(dueSchedule.durationMinutes) +
                 " minutes");
  // Trigger action is intentionally handled by the caller in a later step.
}

bool ScheduledIrrigation::consumeDueSchedule(
    unsigned long currentTime, IrrigationSchedule &dueSchedule)
{
  if (_zoneIndex < 0 || _scheduleCount == 0 || _wallClock == nullptr)
  {
    return false;
  }

  (void)currentTime;

  WallClockTime now;
  if (!_wallClock->getLocalTime(now))
  {
    return false;
  }

  const uint64_t triggerKey = getTriggerKey(now);
  if (_hasLastTriggerKey && triggerKey == _lastTriggerKey)
  {
    return false;
  }

  for (int i = 0; i < _scheduleCount; i++)
  {
    if (_schedules[i].enabled && isScheduleTimeMatch(_schedules[i], now))
    {
      _lastTriggerKey = triggerKey;
      _hasLastTriggerKey = true;
      dueSchedule = _schedules[i];
      return true;
    }
  }

  return false;
}

void ScheduledIrrigation::addSchedule(const IrrigationSchedule &schedule)
{
  if (_scheduleCount < MAX_SCHEDULES_PER_ZONE)
  {
    _schedules[_scheduleCount] = schedule;
    _scheduleCount++;
    Trace::log(TraceLevel::INFO,
               "Schedule added for zone " + String(_zoneIndex));
  }
  else
  {
    Trace::log(TraceLevel::ERROR,
               "Cannot add schedule - max schedules reached for zone " +
                   String(_zoneIndex));
  }
}

void ScheduledIrrigation::removeSchedule(int scheduleIndex)
{
  if (scheduleIndex >= 0 && scheduleIndex < _scheduleCount)
  {
    // Shift schedules down
    for (int i = scheduleIndex; i < _scheduleCount - 1; i++)
    {
      _schedules[i] = _schedules[i + 1];
    }
    _scheduleCount--;
    Trace::log(TraceLevel::INFO,
               "Schedule removed from zone " + String(_zoneIndex));
  }
}

void ScheduledIrrigation::updateSchedule(int scheduleIndex,
                                         const IrrigationSchedule &schedule)
{
  if (scheduleIndex >= 0 && scheduleIndex < _scheduleCount)
  {
    _schedules[scheduleIndex] = schedule;
    Trace::log(TraceLevel::INFO,
               "Schedule updated for zone " + String(_zoneIndex));
  }
}

IrrigationSchedule ScheduledIrrigation::getSchedule(int scheduleIndex) const
{
  if (scheduleIndex >= 0 && scheduleIndex < _scheduleCount)
  {
    return _schedules[scheduleIndex];
  }
  return IrrigationSchedule(); // Return default if invalid
}

int ScheduledIrrigation::getScheduleCount() const
{
  return _scheduleCount;
}

void ScheduledIrrigation::enableSchedule(int scheduleIndex)
{
  if (scheduleIndex >= 0 && scheduleIndex < _scheduleCount)
  {
    _schedules[scheduleIndex].enabled = true;
  }
}

void ScheduledIrrigation::disableSchedule(int scheduleIndex)
{
  if (scheduleIndex >= 0 && scheduleIndex < _scheduleCount)
  {
    _schedules[scheduleIndex].enabled = false;
  }
}

bool ScheduledIrrigation::isScheduleEnabled(int scheduleIndex) const
{
  if (scheduleIndex >= 0 && scheduleIndex < _scheduleCount)
  {
    return _schedules[scheduleIndex].enabled;
  }
  return false;
}

bool ScheduledIrrigation::shouldRunNow(unsigned long currentTime) const
{
  (void)currentTime;
  return shouldRunNow();
}

bool ScheduledIrrigation::shouldRunNow() const
{
  for (int i = 0; i < _scheduleCount; i++)
  {
    if (_schedules[i].enabled && isScheduleTimeMatch(_schedules[i]))
    {
      return true;
    }
  }
  return false;
}

String ScheduledIrrigation::getScheduleCommandTopic() const
{
  // Format: garden_controller/zone0/schedule/command
  return "zone" + String(_zoneIndex) + "/schedule/command";
}

String ScheduledIrrigation::getScheduleStatusTopic() const
{
  // Format: garden_controller/zone0/schedule/status
  return "zone" + String(_zoneIndex) + "/schedule/status";
}

bool ScheduledIrrigation::isScheduleTimeMatch(
    const IrrigationSchedule &schedule) const
{
  if (_wallClock == nullptr)
  {
    return false;
  }

  WallClockTime now;
  if (!_wallClock->getLocalTime(now))
  {
    return false;
  }

  return isScheduleTimeMatch(schedule, now);
}

bool ScheduledIrrigation::isScheduleTimeMatch(
    const IrrigationSchedule &schedule, const WallClockTime &time) const
{
  if (time.dayOfWeek < 0 || time.dayOfWeek > 6)
  {
    return false;
  }

  const uint8_t dayMask = static_cast<uint8_t>(1U << time.dayOfWeek);
  return schedule.hour == time.hour && schedule.minute == time.minute &&
         (schedule.daysOfWeek & dayMask) != 0;
}

uint64_t ScheduledIrrigation::getTriggerKey(const WallClockTime &time) const
{
  return (((((uint64_t)time.year * 100ULL + (uint64_t)time.month) * 100ULL +
            (uint64_t)time.day) *
               100ULL +
           (uint64_t)time.hour) *
              100ULL +
          (uint64_t)time.minute);
}
