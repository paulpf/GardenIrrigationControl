#include "scheduledirrigationpublisher.h"
#include <ArduinoJson.h>

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
    if (!_hasPublished)
    {
      publishZoneSummary(zoneIndex);
    }
  }

  _lastPublishMillis = currentMillis;
  _hasPublished = true;
}

void ScheduledIrrigationPublisher::publishAllNow()
{
  if (!_publisher.isConnected() || _clientName.length() == 0)
  {
    return;
  }

  for (int z = 0; z < MAX_IRRIGATION_ZONES; z++)
  {
    for (int s = 0; s < MAX_SCHEDULES_PER_ZONE; s++)
      publishScheduleStateJson(z, s);
    publishZoneSummary(z);
  }

  _lastPublishMillis = millis();
  _hasPublished = true;
}

void ScheduledIrrigationPublisher::publishScheduleState(int zoneIndex,
                                                        int scheduleIndex)
{
  publishScheduleStateJson(zoneIndex, scheduleIndex);
}

String ScheduledIrrigationPublisher::getScheduleBaseTopic(
    int zoneIndex, int scheduleIndex) const
{
  return _clientName + "/irrigationZone" + String(zoneIndex + 1) +
         "/schedule" + String(scheduleIndex + 1);
}

String ScheduledIrrigationPublisher::getScheduleStateTopic(
    int zoneIndex, int scheduleIndex) const
{
  return getScheduleBaseTopic(zoneIndex, scheduleIndex) + "/state";
}

void ScheduledIrrigationPublisher::publishScheduleStateJson(int zoneIndex,
                                                            int scheduleIndex)
{
  if (!_publisher.isConnected() || _clientName.length() == 0)
  {
    return;
  }

  const bool configured =
      _scheduledIrrigations[zoneIndex].isScheduleConfigured(scheduleIndex);

  JsonDocument doc;
  doc["configured"] = configured;

  if (configured)
  {
    IrrigationSchedule schedule =
        _scheduledIrrigations[zoneIndex].getSchedule(scheduleIndex);
    doc["enabled"]         = schedule.enabled;
    doc["startTime"]       = formatStartTime(schedule.hour, schedule.minute);
    doc["durationMinutes"] = schedule.durationMinutes;
    doc["daysOfWeek"]      = (int)schedule.daysOfWeek;
    doc["daysText"]        = formatDaysText(schedule.daysOfWeek);
  }

  char buffer[256];
  serializeJson(doc, buffer, sizeof(buffer));
  _publisher.publishRetained(getScheduleStateTopic(zoneIndex, scheduleIndex).c_str(),
                             buffer);
}

String ScheduledIrrigationPublisher::getZoneSummaryTopic(int zoneIndex) const
{
  return _clientName + "/irrigationZone" + String(zoneIndex + 1) +
         "/scheduleSummary";
}

String ScheduledIrrigationPublisher::formatStartTime(int hour, int minute) const
{
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "%02d:%02d", hour, minute);
  return String(buffer);
}

String ScheduledIrrigationPublisher::formatDaysText(uint8_t daysOfWeek) const
{
  static const char *labels[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
  String result;
  for (int i = 0; i < 7; i++)
  {
    if (daysOfWeek & (1 << i))
    {
      if (result.length() > 0) result += ',';
      result += labels[i];
    }
  }
  return result.length() > 0 ? result : String("-");
}

void ScheduledIrrigationPublisher::publishZoneSummary(int zoneIndex)
{
  if (!_publisher.isConnected() || _clientName.length() == 0)
  {
    return;
  }

  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int s = 0; s < MAX_SCHEDULES_PER_ZONE; s++)
  {
    if (!_scheduledIrrigations[zoneIndex].isScheduleConfigured(s))
    {
      continue;
    }
    IrrigationSchedule schedule = _scheduledIrrigations[zoneIndex].getSchedule(s);
    JsonObject entry = arr.add<JsonObject>();
    entry["slot"]            = s + 1;
    entry["configured"]      = true;
    entry["enabled"]         = schedule.enabled;
    entry["startTime"]       = formatStartTime(schedule.hour, schedule.minute);
    entry["durationMinutes"] = schedule.durationMinutes;
    entry["daysOfWeek"]      = schedule.daysOfWeek;
    entry["daysText"]        = formatDaysText(schedule.daysOfWeek);
  }

  char buffer[768];
  serializeJson(doc, buffer, sizeof(buffer));
  _publisher.publishRetained(getZoneSummaryTopic(zoneIndex).c_str(), buffer);
}
