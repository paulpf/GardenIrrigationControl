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
  void setDeactivationCallback(const std::function<void()>& callback);
  void setActivationCallback(const std::function<void()>& callback);
  void setTickCallback(const std::function<void()>& callback);

private:
  unsigned long startTime;
  unsigned long duration;
  unsigned long lastTickTime;
  bool active;
  std::function<void()> deactivationCallback;
  std::function<void()> activationCallback;
  std::function<void()> tickCallback;
};

#endif // TIMER_H