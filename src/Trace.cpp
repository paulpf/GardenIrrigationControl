#include "Trace.h"

void Trace::log(String message)
{
  #if TRACE_ENABLED
  String timeStamp = String(millis());
  Serial.println(timeStamp + " | " + message);
  #endif
}