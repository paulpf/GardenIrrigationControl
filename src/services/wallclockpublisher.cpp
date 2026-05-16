#include "wallclockpublisher.h"

WallClockPublisher::WallClockPublisher(IMessagePublisher &publisher,
                                       IWallClock &wallClock)
    : _publisher(publisher), _wallClock(wallClock)
{
}

void WallClockPublisher::setup(const char *clientName)
{
  String baseTopic = String(clientName) + "/system/time";
  _timeValidTopic = baseTopic + "/valid";
  _localTimeTopic = baseTopic + "/local";
  _dayOfWeekTopic = baseTopic + "/dayOfWeek";
}

void WallClockPublisher::publishTime()
{
  if (!_publisher.isConnected() || _timeValidTopic.length() == 0)
  {
    return;
  }

  WallClockTime time;
  const bool hasValidTime = _wallClock.getLocalTime(time);
  _publisher.publish(_timeValidTopic.c_str(), hasValidTime ? "true" : "false");

  if (!hasValidTime)
  {
    return;
  }

  String formattedTime = formatLocalTime(time);
  _publisher.publish(_localTimeTopic.c_str(), formattedTime.c_str());
  _publisher.publish(_dayOfWeekTopic.c_str(), String(time.dayOfWeek).c_str());
}

String WallClockPublisher::formatLocalTime(const WallClockTime &time)
{
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02d",
           time.year, time.month, time.day, time.hour, time.minute,
           time.second);
  return String(buffer);
}
