#ifndef LOOPSCHEDULER_H
#define LOOPSCHEDULER_H

class LoopScheduler
{
public:
  bool shouldRun(unsigned long currentMillis, unsigned long &previousMillis,
                 unsigned long interval) const
  {
    if ((unsigned long)(currentMillis - previousMillis) < interval)
    {
      return false;
    }

    previousMillis = currentMillis;
    return true;
  }
};

#endif // LOOPSCHEDULER_H
