// ITimer.h

#ifndef ITIMER_H
#define ITIMER_H

#include <Arduino.h>

class ITimer
{
  public:
  virtual void setup(IMqttClient* mqttClient, int tmrid) = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void reset() = 0;
  virtual bool isRunning() = 0;
  virtual unsigned long getDuration() = 0;
  virtual void setDuration(unsigned long duration) = 0;
  virtual void loop() = 0;
};

#endif // ITIMER_H