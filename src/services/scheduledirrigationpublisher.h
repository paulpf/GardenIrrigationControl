#ifndef SCHEDULEDIRRIGATIONPUBLISHER_H
#define SCHEDULEDIRRIGATIONPUBLISHER_H

#include <Arduino.h>

#include "imessagepublisher.h"
#include "scheduled_irrigation.h"

class ScheduledIrrigationPublisher
{
public:
  ScheduledIrrigationPublisher(
      IMessagePublisher &publisher,
      ScheduledIrrigation (&scheduledIrrigations)[MAX_IRRIGATION_ZONES]);

  void setup(const char *clientName);
  void publishState(unsigned long currentMillis);
  void publishScheduleState(int zoneIndex, int scheduleIndex);

private:
  static constexpr unsigned long PUBLISH_INTERVAL_MS = 60000;

  IMessagePublisher &_publisher;
  ScheduledIrrigation (&_scheduledIrrigations)[MAX_IRRIGATION_ZONES];
  String _clientName;
  unsigned long _lastPublishMillis = 0;
  bool _hasPublished = false;

  String getScheduleBaseTopic(int zoneIndex, int scheduleIndex) const;
};

#endif // SCHEDULEDIRRIGATIONPUBLISHER_H
