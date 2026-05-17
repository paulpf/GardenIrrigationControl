#ifndef SCHEDULEDIRRIGATIONCOMMANDHANDLER_H
#define SCHEDULEDIRRIGATIONCOMMANDHANDLER_H

#include <Arduino.h>

#include "irrigationconfig.h"
#include "scheduled_irrigation.h"
#include "scheduledirrigationpublisher.h"

class ScheduledIrrigationCommandHandler
{
public:
  ScheduledIrrigationCommandHandler(
      ScheduledIrrigation (&scheduledIrrigations)[MAX_IRRIGATION_ZONES],
      ScheduledIrrigationPublisher &publisher,
      IrrigationConfig &irrigationConfig);

  void setup(const char *clientName);
  bool handleCommand(const char *topic, const char *payload);

private:
  ScheduledIrrigation (&_scheduledIrrigations)[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher &_publisher;
  IrrigationConfig &_irrigationConfig;
  String _clientName;

  bool matchesFieldTopic(const char *topic, int zoneIndex, int scheduleIndex,
                         const char *field) const;
  bool parseBoolPayload(const char *payload, bool &value) const;
  bool parseIntPayload(const char *payload, int &value) const;
  bool updateBoolField(int zoneIndex, int scheduleIndex, const char *field,
                       bool value);
  bool updateIntField(int zoneIndex, int scheduleIndex, const char *field,
                      int value);
  bool ensureSchedule(int zoneIndex, int scheduleIndex,
                      IrrigationSchedule &schedule);
  void saveAndPublish(int zoneIndex, int scheduleIndex,
                      const IrrigationSchedule &schedule);
  void clearAndPublish(int zoneIndex, int scheduleIndex);
};

#endif // SCHEDULEDIRRIGATIONCOMMANDHANDLER_H
