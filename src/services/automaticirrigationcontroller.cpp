#include "automaticirrigationcontroller.h"

void AutomaticIrrigationController::setup(bool enabledByDefault)
{
  _enabled = enabledByDefault;
}

void AutomaticIrrigationController::setEnabled(bool enabled)
{
  _enabled = enabled;
}

bool AutomaticIrrigationController::isEnabled() const
{
  return _enabled;
}

bool AutomaticIrrigationController::canRunSchedules() const
{
  return _enabled;
}
