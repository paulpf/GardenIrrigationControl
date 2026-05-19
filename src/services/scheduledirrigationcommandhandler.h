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
  void resetAll();

private:
  ScheduledIrrigation (&_scheduledIrrigations)[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher &_publisher;
  IrrigationConfig &_irrigationConfig;
  String _clientName;

  bool matchesFieldTopic(const char *topic, int zoneIndex, int scheduleIndex,
                         const char *field) const;
  bool parseBoolPayload(const char *payload, bool &value) const;
  bool parseIntPayload(const char *payload, int &value) const;
  bool parseStartTimePayload(const char *payload, int &hour, int &minute) const;
  bool updateBoolField(int zoneIndex, int scheduleIndex, const char *field,
                       bool value);
  bool updateStartTimeField(int zoneIndex, int scheduleIndex,
                            const char *payload);
  bool updateIntField(int zoneIndex, int scheduleIndex, const char *field,
                      int value);
  bool ensureSchedule(int zoneIndex, int scheduleIndex,
                      IrrigationSchedule &schedule,
                      bool createIfMissing = false);
  void saveAndPublish(int zoneIndex, int scheduleIndex,
                      const IrrigationSchedule &schedule);
  void clearAndPublish(int zoneIndex, int scheduleIndex);
};

#endif // SCHEDULEDIRRIGATIONCOMMANDHANDLER_H
