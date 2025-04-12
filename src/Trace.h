#ifndef TRACE_H
#define TRACE_H

#include <Arduino.h>

class Trace
{
public:
  static void log(String message);
};

#endif // TRACE_H