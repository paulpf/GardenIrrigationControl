#ifndef WALLCLOCKPUBLISHER_H
#define WALLCLOCKPUBLISHER_H

#include <Arduino.h>

#include "imessagepublisher.h"
#include "iwallclock.h"

class WallClockPublisher
{
public:
  WallClockPublisher(IMessagePublisher &publisher, IWallClock &wallClock);

  void setup(const char *clientName);
  void publishTime();

private:
  IMessagePublisher &_publisher;
  IWallClock &_wallClock;
  String _timeValidTopic;
  String _localTimeTopic;
  String _dayOfWeekTopic;

  static String formatLocalTime(const WallClockTime &time);
};

#endif // WALLCLOCKPUBLISHER_H
