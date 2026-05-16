#ifndef SCHEDULED_IRRIGATION_H
#define SCHEDULED_IRRIGATION_H

#include "global_defines.h"

// NOTE: ScheduledIrrigation is NOT yet functional.
// isScheduleTimeMatch() returns false unconditionally until NTP integration
// is implemented. Do not treat this as a production-ready feature.

// Schedule structure for one irrigation event
struct IrrigationSchedule
{
  bool enabled;        // Is this schedule active?
  int hour;            // 0-23 (UTC)
  int minute;          // 0-59
  int durationMinutes; // How long to run (1-60 minutes)
  uint8_t daysOfWeek;  // Bitmask: bit0=Mon, bit1=Tue, ... bit6=Sun

  IrrigationSchedule()
      : enabled(false), hour(6), minute(0), durationMinutes(5), daysOfWeek(0x3E)
  {
  } // Default: Mon-Fri 6:00 AM, 5 min
};

// Maximum schedules per zone
constexpr int MAX_SCHEDULES_PER_ZONE = 4;

class ScheduledIrrigation
{
public:
  ScheduledIrrigation();

  // Setup with zone index and MQTT manager reference
  void setup(int zoneIndex);

  // Call this frequently to check if irrigation should trigger
  void loop(unsigned long currentTime);

  // Schedule management
  void addSchedule(const IrrigationSchedule &schedule);
  void removeSchedule(int scheduleIndex);
  void updateSchedule(int scheduleIndex, const IrrigationSchedule &schedule);
  IrrigationSchedule getSchedule(int scheduleIndex) const;
  int getScheduleCount() const;

  // Enable/disable specific schedule
  void enableSchedule(int scheduleIndex);
  void disableSchedule(int scheduleIndex);
  bool isScheduleEnabled(int scheduleIndex) const;

  // Check if it's time to run
  bool shouldRunNow(unsigned long currentTime) const;

  // MQTT topics for schedule commands
  String getScheduleCommandTopic() const;
  String getScheduleStatusTopic() const;

private:
  int _zoneIndex;
  IrrigationSchedule _schedules[MAX_SCHEDULES_PER_ZONE];
  int _scheduleCount;
  unsigned long _lastTriggerTime; // Prevent multiple triggers in same minute

  bool isScheduleTimeMatch(const IrrigationSchedule &schedule,
                           unsigned long currentTime) const;
  int getDayOfWeek(unsigned long currentTime) const; // 0=Mon, 6=Sun
};

#endif // SCHEDULED_IRRIGATION_H
