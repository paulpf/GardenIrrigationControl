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

  _publisher.publish((baseTopic + "/configured").c_str(),
                     configured ? "true" : "false");
  _publisher.publish((baseTopic + "/enabled").c_str(),
                     schedule.enabled ? "true" : "false");
  _publisher.publish((baseTopic + "/hour").c_str(),
                     String(schedule.hour).c_str());
  _publisher.publish((baseTopic + "/minute").c_str(),
                     String(schedule.minute).c_str());
  _publisher.publish((baseTopic + "/daysOfWeek").c_str(),
                     String((int)schedule.daysOfWeek).c_str());
  _publisher.publish((baseTopic + "/durationMinutes").c_str(),
                     String(schedule.durationMinutes).c_str());
}

String ScheduledIrrigationPublisher::getScheduleBaseTopic(
    int zoneIndex, int scheduleIndex) const
{
  return _clientName + "/irrigationZone" + String(zoneIndex + 1) +
         "/schedule" + String(scheduleIndex + 1);
}
