#ifndef TIMER_H
#define TIMER_H

#include "GlobalDefines.h"

class Timer {
public:
  Timer();
  void start(unsigned long duration);
  void stop();
  void update();
  bool isActive() const;
  unsigned long getRemainingTime() const;
  void handleTick();
  void setDeactivationCallback(void (*callback)());
  void setActivationCallback(void (*callback)());
  void setTickCallback(void (*callback)());

private:
  unsigned long startTime;
  unsigned long duration;
  unsigned long lastTickTime;
  bool active;
  void (*deactivationCallback)();
  void (*activationCallback)();
  void (*tickCallback)();
};

#endif // TIMER_H