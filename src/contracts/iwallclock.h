#ifndef IWALLCLOCK_H
#define IWALLCLOCK_H

struct WallClockTime
{
  int year = 0;
  int month = 0;
  int day = 0;
  int hour = 0;
  int minute = 0;
  int second = 0;
  int dayOfWeek = 0; // 0=Monday, 6=Sunday
};

class IWallClock
{
public:
  virtual ~IWallClock() = default;

  virtual void setup() = 0;
  virtual bool isTimeValid() const = 0;
  virtual bool getLocalTime(WallClockTime &time) const = 0;
};

#endif // IWALLCLOCK_H
