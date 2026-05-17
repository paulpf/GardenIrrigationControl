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

  const char *boolFields[] = {"configured", "enabled", "delete"};
  const char *intFields[] = {"hour", "minute", "durationMinutes",
                             "daysOfWeek"};

  for (int zoneIndex = 0; zoneIndex < MAX_IRRIGATION_ZONES; zoneIndex++)
  {
    for (int scheduleIndex = 0; scheduleIndex < MAX_SCHEDULES_PER_ZONE;
         scheduleIndex++)
    {
      for (size_t fieldIndex = 0; fieldIndex < sizeof(boolFields) /
                                                   sizeof(boolFields[0]);
           fieldIndex++)
      {
        if (!matchesFieldTopic(topic, zoneIndex, scheduleIndex,
                               boolFields[fieldIndex]))
        {
          continue;
        }

        bool value = false;
        if (parseBoolPayload(payload, value))
        {
          updateBoolField(zoneIndex, scheduleIndex, boolFields[fieldIndex],
                          value);
        }
        return true;
      }

      for (size_t fieldIndex = 0; fieldIndex < sizeof(intFields) /
                                                   sizeof(intFields[0]);
           fieldIndex++)
      {
        if (!matchesFieldTopic(topic, zoneIndex, scheduleIndex,
                               intFields[fieldIndex]))
        {
          continue;
        }

        int value = 0;
        if (parseIntPayload(payload, value))
        {
          updateIntField(zoneIndex, scheduleIndex, intFields[fieldIndex],
                         value);
        }
        return true;
      }
    }
  }

  return false;
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

  if (strcmp(field, "configured") == 0 && !value)
  {
    clearAndPublish(zoneIndex, scheduleIndex);
    return true;
  }

  IrrigationSchedule schedule;
  if (!ensureSchedule(zoneIndex, scheduleIndex, schedule))
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
  if (!ensureSchedule(zoneIndex, scheduleIndex, schedule))
  {
    return false;
  }

  if (strcmp(field, "hour") == 0)
  {
    if (value < 0 || value > 23)
    {
      return false;
    }
    schedule.hour = value;
  }
  else if (strcmp(field, "minute") == 0)
  {
    if (value < 0 || value > 59)
    {
      return false;
    }
    schedule.minute = value;
  }
  else if (strcmp(field, "durationMinutes") == 0)
  {
    const int maxDurationMinutes = _irrigationConfig.maxDurationMs / 60000;
    if (value < 1 || value > maxDurationMinutes)
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

bool ScheduledIrrigationCommandHandler::ensureSchedule(
    int zoneIndex, int scheduleIndex, IrrigationSchedule &schedule)
{
  if (zoneIndex < 0 || zoneIndex >= MAX_IRRIGATION_ZONES ||
      scheduleIndex < 0 || scheduleIndex >= MAX_SCHEDULES_PER_ZONE)
  {
    return false;
  }

  if (_scheduledIrrigations[zoneIndex].isScheduleConfigured(scheduleIndex))
  {
    schedule = _scheduledIrrigations[zoneIndex].getSchedule(scheduleIndex);
  }
  else
  {
    schedule = IrrigationSchedule();
  }

  return true;
}

void ScheduledIrrigationCommandHandler::saveAndPublish(
    int zoneIndex, int scheduleIndex, const IrrigationSchedule &schedule)
{
  _scheduledIrrigations[zoneIndex].updateSchedule(scheduleIndex, schedule);
  StorageManager::getInstance().saveSchedule(zoneIndex, scheduleIndex,
                                             schedule);
  _publisher.publishScheduleState(zoneIndex, scheduleIndex);
}

void ScheduledIrrigationCommandHandler::clearAndPublish(int zoneIndex,
                                                        int scheduleIndex)
{
  _scheduledIrrigations[zoneIndex].removeSchedule(scheduleIndex);
  StorageManager::getInstance().clearSchedule(zoneIndex, scheduleIndex);
  _publisher.publishScheduleState(zoneIndex, scheduleIndex);
}
