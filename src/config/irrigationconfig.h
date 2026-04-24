#ifndef IRRIGATIONCONFIG_H
#define IRRIGATIONCONFIG_H

#include "config.h"

struct IrrigationConfig
{
  // Default duration for irrigation cycles
  unsigned long defaultDurationMs = DEFAULT_DURATION_TIME;
  
  // Maximum allowed duration for a single irrigation cycle
  unsigned long maxDurationMs = MAX_DURATION_TIME;
  
  // Button debounce time to prevent accidental re-triggering
  unsigned long buttonDebounceMs = BUTTON_DEBOUNCE_TIME;
};

#endif // IRRIGATIONCONFIG_H
