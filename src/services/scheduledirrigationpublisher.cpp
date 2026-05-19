#include "scheduledirrigationpublisher.h"

ScheduledIrrigationPublisher::ScheduledIrrigationPublisher(
    IMessagePublisher &publisher,
    ScheduledIrrigation (&scheduledIrrigations)[MAX_IRRIGATION_ZONES])
    : _publisher(publisher), _scheduledIrrigations(scheduledIrrigations)
{
}

void ScheduledIrrigationPublisher::setup(const char *clientName)
{
  _clientName = clientName;
}

void ScheduledIrrigationPublisher::publishState(unsigned long currentMillis)
{
  if (!_publisher.isConnected() || _clientName.length() == 0)
  {
    return;
  }

  if (_hasPublished &&
      (unsigned long)(currentMillis - _lastPublishMillis) < PUBLISH_INTERVAL_MS)
  {
    return;
  }

  for (int zoneIndex = 0; zoneIndex < MAX_IRRIGATION_ZONES; zoneIndex++)
  {
    for (int scheduleIndex = 0; scheduleIndex < MAX_SCHEDULES_PER_ZONE;
         scheduleIndex++)
    {
      publishScheduleState(zoneIndex, scheduleIndex);
    }
  }

  _lastPublishMillis = currentMillis;
  _hasPublished = true;
}

void ScheduledIrrigationPublisher::publishScheduleState(int zoneIndex,
                                                        int scheduleIndex)
{
  if (!_publisher.isConnected() || _clientName.length() == 0)
  {
    return;
  }

  const bool configured =
      _scheduledIrrigations[zoneIndex].isScheduleConfigured(scheduleIndex);
  IrrigationSchedule schedule =
      configured ? _scheduledIrrigations[zoneIndex].getSchedule(scheduleIndex)
                 : IrrigationSchedule();

  String baseTopic = getScheduleBaseTopic(zoneIndex, scheduleIndex);

  const char *configuredVal = configured ? "true" : "false";
  const char *enabledVal = schedule.enabled ? "true" : "false";
  String startTime = formatStartTime(schedule.hour, schedule.minute);
  String daysOfWeek = String((int)schedule.daysOfWeek);
  String durationMinutes = String(schedule.durationMinutes);

  _publisher.publish((baseTopic + "/configured").c_str(), configuredVal);
  _publisher.publish((baseTopic + "/configured/set").c_str(), configuredVal);
  _publisher.publish((baseTopic + "/enabled").c_str(), enabledVal);
  _publisher.publish((baseTopic + "/enabled/set").c_str(), enabledVal);
  _publisher.publish((baseTopic + "/startTime").c_str(), startTime.c_str());
  _publisher.publish((baseTopic + "/startTime/set").c_str(), startTime.c_str());
  _publisher.publish((baseTopic + "/daysOfWeek").c_str(), daysOfWeek.c_str());
  _publisher.publish((baseTopic + "/daysOfWeek/set").c_str(), daysOfWeek.c_str());
  _publisher.publish((baseTopic + "/durationMinutes").c_str(),
                     durationMinutes.c_str());
  _publisher.publish((baseTopic + "/durationMinutes/set").c_str(),
                     durationMinutes.c_str());
}

String ScheduledIrrigationPublisher::getScheduleBaseTopic(
    int zoneIndex, int scheduleIndex) const
{
  return _clientName + "/irrigationZone" + String(zoneIndex + 1) +
         "/schedule" + String(scheduleIndex + 1);
}

String ScheduledIrrigationPublisher::formatStartTime(int hour, int minute) const
{
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
  return String(buffer);
}
