#ifndef WATERLEVELMANAGER_H
#define WATERLEVELMANAGER_H

#include "global_defines.h"
#include "imessagepublisher.h"

class WaterLevelManager
{
public:
  explicit WaterLevelManager(IMessagePublisher &messagePublisher);

  void setup(const char *clientName);
  void loop(unsigned long currentMillis);

private:
  struct State
  {
    int rawValue = 0;
    float percent = 0.0f;
    float liters = 0.0f;
    float overflowLiters = 0.0f;
    float litersToOverflow = 0.0f;
    bool lowWaterLockoutActive = false;
    bool criticalOverflowAlarmActive = false;
    bool overflowActive = false;
    bool safetyLockActive = false;
  };

  struct Topics
  {
    String percent;
    String liters;
    String raw;
    String status;
    String lockout;
    String criticalHighAlarm;
    String overflow;
    String safetyLock;
    String overflowLiters;
    String litersToOverflow;
  };

  struct TransitionEvents
  {
    bool lowWaterLockoutChanged = false;
    bool criticalOverflowAlarmChanged = false;
    bool overflowChanged = false;
    bool safetyLockChanged = false;
  };

  IMessagePublisher &_messagePublisher;
  unsigned long _previousRead = 0;
  State _state;
  Topics _topics;

  void initTopics(const char *clientName);
  void initSensor();
  bool shouldRead(unsigned long currentMillis);
  void updateMetrics();
  void logSnapshot() const;
  TransitionEvents updateStateTransitions();
  void applyTransitionEffects(const TransitionEvents &events);
  void publishStateChange(const char *topic, bool active);
  void publishData();
};

#endif // WATERLEVELMANAGER_H