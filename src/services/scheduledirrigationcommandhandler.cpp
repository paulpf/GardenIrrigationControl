#include "scheduledirrigationcommandhandler.h"

#include "storage_manager.h"
#include "trace.h"
#include <climits>
#include <cstring>

ScheduledIrrigationCommandHandler::ScheduledIrrigationCommandHandler(
    ScheduledIrrigation (&scheduledIrrigations)[MAX_IRRIGATION_ZONES],
    ScheduledIrrigationPublisher &publisher, IrrigationConfig &irrigationConfig)
    : _scheduledIrrigations(scheduledIrrigations), _publisher(publisher),
      _irrigationConfig(irrigationConfig)
{
}

void ScheduledIrrigationCommandHandler::setup(const char *clientName)
{
  _clientName = clientName;
}

bool ScheduledIrrigationCommandHandler::handleCommand(const char *topic,
                                                      const char *payload)
{
  if (topic == nullptr || payload == nullptr || _clientName.length() == 0)
  {
    return false;
  }

  for (int zoneIndex = 0; zoneIndex < MAX_IRRIGATION_ZONES; zoneIndex++)
  {
    for (int scheduleIndex = 0; scheduleIndex < MAX_SCHEDULES_PER_ZONE;
         scheduleIndex++)
    {
      if (matchesJsonSetTopic(topic, zoneIndex, scheduleIndex))
      {
        handleJsonCommand(zoneIndex, scheduleIndex, payload);
        return true;
      }

      if (matchesFieldTopic(topic, zoneIndex, scheduleIndex, "delete"))
      {
        bool value = false;
        if (parseBoolPayload(payload, value) && value)
        {
          clearAndPublish(zoneIndex, scheduleIndex);
        }
        return true;
      }
    }
  }

  return false;
}

bool ScheduledIrrigationCommandHandler::matchesJsonSetTopic(
    const char *topic, int zoneIndex, int scheduleIndex) const
{
  char expected[128];
  snprintf(expected, sizeof(expected), "%s/irrigationZone%d/schedule%d/set",
           _clientName.c_str(), zoneIndex + 1, scheduleIndex + 1);
  return strcmp(topic, expected) == 0;
}

bool ScheduledIrrigationCommandHandler::handleJsonCommand(
    int zoneIndex, int scheduleIndex, const char *payload)
{
  if (payload == nullptr || payload[0] == '\0')
  {
    return true;
  }

  JsonDocument doc;
  if (deserializeJson(doc, payload) != DeserializationError::Ok)
  {
    Trace::log(TraceLevel::ERROR,
               "Invalid JSON for schedule command: " + String(payload));
    return false;
  }

  if (doc["configured"] == false)
  {
    clearAndPublish(zoneIndex, scheduleIndex);
    return true;
  }

  IrrigationSchedule schedule;
  ensureSchedule(zoneIndex, scheduleIndex, schedule, true);

  if (doc.containsKey("enabled"))
    schedule.enabled = doc["enabled"].as<bool>();

  if (doc.containsKey("startTime"))
  {
    int hour = 0, minute = 0;
    if (parseStartTimePayload(doc["startTime"].as<const char *>(), hour, minute))
    {
      schedule.hour = hour;
      schedule.minute = minute;
    }
  }

  if (doc.containsKey("durationMinutes"))
    schedule.durationMinutes = doc["durationMinutes"].as<int>();

  if (doc.containsKey("daysOfWeek"))
    schedule.daysOfWeek = (uint8_t)doc["daysOfWeek"].as<int>();

  saveAndPublish(zoneIndex, scheduleIndex, schedule);
  return true;
}

bool ScheduledIrrigationCommandHandler::matchesFieldTopic(
    const char *topic, int zoneIndex, int scheduleIndex, const char *field) const
{
  char expected[128];
  snprintf(expected, sizeof(expected), "%s/irrigationZone%d/schedule%d/%s/set",
           _clientName.c_str(), zoneIndex + 1, scheduleIndex + 1, field);
  return strcmp(topic, expected) == 0;
}

bool ScheduledIrrigationCommandHandler::parseBoolPayload(const char *payload,
                                                         bool &value) const
{
  if (strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0 ||
      strcmp(payload, "on") == 0)
  {
    value = true;
    return true;
  }

  if (strcmp(payload, "false") == 0 || strcmp(payload, "0") == 0 ||
      strcmp(payload, "off") == 0)
  {
    value = false;
    return true;
  }

  return false;
}

bool ScheduledIrrigationCommandHandler::parseIntPayload(const char *payload,
                                                        int &value) const
{
  if (payload[0] == '\0')
  {
    return false;
  }

  long parsed = 0;
  for (const char *cursor = payload; *cursor != '\0'; cursor++)
  {
    if (*cursor < '0' || *cursor > '9')
    {
      return false;
    }

    parsed = parsed * 10 + (*cursor - '0');
    if (parsed > INT_MAX)
    {
      return false;
    }
  }

  value = (int)parsed;
  return true;
}

bool ScheduledIrrigationCommandHandler::parseStartTimePayload(
    const char *payload, int &hour, int &minute) const
{
  if (payload == nullptr || strlen(payload) != 5 || payload[2] != ':')
  {
    return false;
  }

  if (payload[0] < '0' || payload[0] > '9' || payload[1] < '0' ||
      payload[1] > '9' || payload[3] < '0' || payload[3] > '9' ||
      payload[4] < '0' || payload[4] > '9')
  {
    return false;
  }

  hour = (payload[0] - '0') * 10 + (payload[1] - '0');
  minute = (payload[3] - '0') * 10 + (payload[4] - '0');

  return hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59;
}

bool ScheduledIrrigationCommandHandler::updateBoolField(
    int zoneIndex, int scheduleIndex, const char *field, bool value)
{
  if (strcmp(field, "delete") == 0)
  {
    if (value)
    {
      clearAndPublish(zoneIndex, scheduleIndex);
    }
    return true;
  }

  if (strcmp(field, "configured") == 0)
  {
    if (!value)
    {
      clearAndPublish(zoneIndex, scheduleIndex);
      return true;
    }

    IrrigationSchedule schedule;
    if (!ensureSchedule(zoneIndex, scheduleIndex, schedule, true))
    {
      return false;
    }

    saveAndPublish(zoneIndex, scheduleIndex, schedule);
    return true;
  }

  IrrigationSchedule schedule;
  if (!ensureSchedule(zoneIndex, scheduleIndex, schedule, false))
  {
    return false;
  }

  if (strcmp(field, "enabled") == 0)
  {
    schedule.enabled = value;
  }

  saveAndPublish(zoneIndex, scheduleIndex, schedule);
  return true;
}

bool ScheduledIrrigationCommandHandler::updateIntField(
    int zoneIndex, int scheduleIndex, const char *field, int value)
{
  IrrigationSchedule schedule;
  if (!ensureSchedule(zoneIndex, scheduleIndex, schedule, false))
  {
    return false;
  }

  if (strcmp(field, "durationMinutes") == 0)
  {
    if (value < 1 || value > INT_MAX / 60000)
    {
      return false;
    }
    schedule.durationMinutes = value;
  }
  else if (strcmp(field, "daysOfWeek") == 0)
  {
    if (value < 1 || value > 127)
    {
      return false;
    }
    schedule.daysOfWeek = (uint8_t)value;
  }

  saveAndPublish(zoneIndex, scheduleIndex, schedule);
  return true;
}

bool ScheduledIrrigationCommandHandler::updateStartTimeField(
    int zoneIndex, int scheduleIndex, const char *payload)
{
  int hour = 0;
  int minute = 0;
  if (!parseStartTimePayload(payload, hour, minute))
  {
    return false;
  }

  IrrigationSchedule schedule;
  if (!ensureSchedule(zoneIndex, scheduleIndex, schedule, false))
  {
    return false;
  }

  schedule.hour = hour;
  schedule.minute = minute;
  saveAndPublish(zoneIndex, scheduleIndex, schedule);
  return true;
}

bool ScheduledIrrigationCommandHandler::ensureSchedule(
  int zoneIndex, int scheduleIndex, IrrigationSchedule &schedule,
  bool createIfMissing)
{
  if (zoneIndex < 0 || zoneIndex >= MAX_IRRIGATION_ZONES ||
      scheduleIndex < 0 || scheduleIndex >= MAX_SCHEDULES_PER_ZONE)
  {
    return false;
  }

  if (_scheduledIrrigations[zoneIndex].isScheduleConfigured(scheduleIndex))
  {
    schedule = _scheduledIrrigations[zoneIndex].getSchedule(scheduleIndex);
    return true;
  }

  if (!createIfMissing)
  {
    return false;
  }

  schedule = IrrigationSchedule();
  return true;
}

void ScheduledIrrigationCommandHandler::saveAndPublish(
    int zoneIndex, int scheduleIndex, const IrrigationSchedule &schedule)
{
  _scheduledIrrigations[zoneIndex].updateSchedule(scheduleIndex, schedule);
  StorageManager::getInstance().saveSchedule(zoneIndex, scheduleIndex,
                                             schedule);
  _publisher.publishScheduleState(zoneIndex, scheduleIndex);
  _publisher.publishZoneSummary(zoneIndex);
}

void ScheduledIrrigationCommandHandler::clearAndPublish(int zoneIndex,
                                                        int scheduleIndex)
{
  _scheduledIrrigations[zoneIndex].removeSchedule(scheduleIndex);
  StorageManager::getInstance().clearSchedule(zoneIndex, scheduleIndex);
  _publisher.publishScheduleState(zoneIndex, scheduleIndex);
  _publisher.publishZoneSummary(zoneIndex);
}

void ScheduledIrrigationCommandHandler::resetAll()
{
  for (int z = 0; z < MAX_IRRIGATION_ZONES; z++)
  {
    for (int s = 0; s < MAX_SCHEDULES_PER_ZONE; s++)
    {
      _scheduledIrrigations[z].removeSchedule(s);
      _publisher.publishScheduleState(z, s);
    }
  }
}
