#include "ntpwallclock.h"

#include "config.h"
#include "trace.h"

#include <Arduino.h>
#include <time.h>

namespace
{
constexpr time_t MIN_VALID_EPOCH = 1704067200; // 2024-01-01 00:00:00 UTC
}

void NtpWallClock::setup()
{
  if (!ENABLE_NTP_TIME_SYNC)
  {
    Trace::log(TraceLevel::INFO, "NTP time sync disabled in configuration");
    return;
  }

  configTzTime(NTP_TIME_ZONE, NTP_SERVER_1, NTP_SERVER_2, NTP_SERVER_3);
  Trace::log(TraceLevel::INFO, "NTP time sync configured");
}

bool NtpWallClock::isTimeValid() const
{
  return time(nullptr) >= MIN_VALID_EPOCH;
}

bool NtpWallClock::getLocalTime(WallClockTime &time) const
{
  if (!isTimeValid())
  {
    return false;
  }

  struct tm localTime;
  if (!::getLocalTime(&localTime, 0))
  {
    return false;
  }

  time.year = localTime.tm_year + 1900;
  time.month = localTime.tm_mon + 1;
  time.day = localTime.tm_mday;
  time.hour = localTime.tm_hour;
  time.minute = localTime.tm_min;
  time.second = localTime.tm_sec;
  time.dayOfWeek = convertToMondayBasedWeekday(localTime.tm_wday);
  return true;
}

int NtpWallClock::convertToMondayBasedWeekday(int tmWeekday)
{
  return tmWeekday == 0 ? 6 : tmWeekday - 1;
}
