#pragma once

#include "irrigation_zone.h"
#include "trace.h"

bool IrrigationZone::_globalStartInhibit = false;

inline void IrrigationZone::setGlobalStartInhibit(bool inhibit)
{
  _globalStartInhibit = inhibit;
}

inline bool IrrigationZone::isGlobalStartInhibited()
{
  return _globalStartInhibit;
}

inline void Trace::log(TraceLevel, String)
{
}

inline void Trace::plotBoolState(String, bool, int)
{
}

inline void Trace::plotLoopTime(String, int, unsigned long)
{
}
