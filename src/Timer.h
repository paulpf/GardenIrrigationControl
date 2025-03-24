#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
public:
  Timer();
  void start(unsigned long duration);
  void stop();
  void update();
  bool isActive() const;
  unsigned long getRemainingTime() const;
  void setDeactivationCallback(void (*callback)());
  void setActivationCallback(void (*callback)());

private:
  unsigned long startTime;
  unsigned long duration;
  bool active;
  void (*deactivationCallback)();
  void (*activationCallback)();
};

#endif // TIMER_H