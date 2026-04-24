#ifndef OTALOOPGUARD_H
#define OTALOOPGUARD_H

#include "iotaloopcontrol.h"

class OtaLoopGuard
{
public:
  explicit OtaLoopGuard(IOtaLoopControl &ota) : _ota(ota)
  {
  }

  bool process()
  {
    _ota.loop();
    return _ota.isUpdating();
  }

private:
  IOtaLoopControl &_ota;
};

#endif // OTALOOPGUARD_H
