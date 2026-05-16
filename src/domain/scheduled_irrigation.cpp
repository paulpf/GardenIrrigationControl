#include "scheduled_irrigation.h"
#include "trace.h"

ScheduledIrrigation::ScheduledIrrigation()
    : _zoneIndex(-1), _scheduleCount(0), _lastTriggerTime(0)
{
}

void ScheduledIrrigation::setup(int zoneIndex)
{
  _zoneIndex = zoneIndex;
  _scheduleCount = 0;
  _lastTriggerTime = 0;
  Trace::log(TraceLevel::INFO,
             "ScheduledIrrigation initialized for zone " + String(zoneIndex));
}

void ScheduledIrrigation::loop(unsigned long currentTime)
{
  if (_zoneIndex < 0 || _scheduleCount == 0)
  {
    return; // Not initialized or no schedules
  }

  // Check all enabled schedules
  for (int i = 0; i < _scheduleCount; i++)
  {
    if (_schedules[i].enabled &&
        isScheduleTimeMatch(_schedules[i], currentTime))
    {
      // Check if enough time has passed since last trigger (prevent
      // retriggering in same minute)
      if ((currentTime - _lastTriggerTime) > 60000) // More than 1 minute
      {
        _lastTriggerTime = currentTime;
        Trace::log(TraceLevel::INFO,
                   "Schedule triggered for zone " + String(_zoneIndex) +
                       " - running for " +
                       String(_schedules[i].durationMinutes) + " minutes");
        // Trigger action would be handled by caller (e.g., start irrigation
        // zone)
      }
    }
  }
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
  for (int i = 0; i < _scheduleCount; i++)
  {
    if (_schedules[i].enabled &&
        isScheduleTimeMatch(_schedules[i], currentTime))
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
    const IrrigationSchedule &schedule, unsigned long currentTime) const
{
  // NOT IMPLEMENTED: requires NTP integration.
  // Always returns false until a real time source is wired in.
  return false;
}

int ScheduledIrrigation::getDayOfWeek(unsigned long currentTime) const
{
  // Convert Unix timestamp to day of week (0=Monday, 6=Sunday)
  // This requires time library integration
  // TODO: Implement with proper time library
  return 0;
}
