// Timer.h

#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include "IMqttClient.h"
#include "ITimer.h"


class Timer : ITimer
{
public:
  Timer();
  ~Timer();
  void setup(IMqttClient* mqttClient, int tmrid);
  void start();
  void stop();
  void reset();
  bool isRunning();
  unsigned long getDuration();
  void setDuration(unsigned long duration);
  void loop();

private:
  IMqttClient* _mqttClient;
  int _tmrid;
  unsigned long _startTime;
  unsigned long _duration;
  bool _running;
};

#endif // TIMER_H
