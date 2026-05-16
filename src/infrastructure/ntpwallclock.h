#ifndef NTPWALLCLOCK_H
#define NTPWALLCLOCK_H

#include "iwallclock.h"

class NtpWallClock : public IWallClock
{
public:
  void setup() override;
  bool isTimeValid() const override;
  bool getLocalTime(WallClockTime &time) const override;

private:
  static int convertToMondayBasedWeekday(int tmWeekday);
};

#endif // NTPWALLCLOCK_H
