// Implementation example for Trace.cpp
#include "Trace.h"

void Trace::log(TraceLevel level, String message) 
{
  // Only log if the message level is >= the configured trace level
  if (!shouldLog(level)) 
  {
    return;
  }
  
  String prefix;
  switch (level) 
  {
    case TraceLevel::INFO:
      prefix = "[INFO] ";
      break;
    case TraceLevel::DEBUG:
      prefix = "[DEBUG] ";
      break;
    case TraceLevel::ERROR:
      prefix = "[ERROR] ";
      break;
  }
  
  Serial.println(prefix + message);
}