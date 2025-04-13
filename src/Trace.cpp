#include "Trace.h"

//#define TRACE

void Trace::log(String message)
{
  #ifdef TRACE
  String timeStamp = String(millis());
  Serial.println(timeStamp + " | " + message);
  #endif
}