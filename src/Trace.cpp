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

// Plotting function for debugging purposes
// This function will print the state of each zone to the Serial Monitor
// It is only enabled if ENABLE_ZONE_PLOTTING is defined
// Visualizing with the Serial Plotter in Arduino IDE or PlatformIO
void Trace::plotBoolState(String stateName, bool stateValue, int valueToPlotForTrue) 
{
    #ifdef ENABLE_ZONE_PLOTTING
    Serial.print(">BoolState_" + stateName + ":");
    Serial.print(stateValue ? String(valueToPlotForTrue) : "0");
    Serial.println(" | np");
    #endif
}
    
// Method to plot loop time for debugging purposes
void Trace::plotLoopTime(String loopName, int loopNameindex, unsigned long loopTime) 
{
    #ifdef ENABLE_LOOP_TIME_PLOTTING
    Serial.print(">LoopTime_" + loopName + "_" + String(loopNameindex));
    Serial.print(":");
    Serial.print(loopTime);
    Serial.println(" | np");
    #endif
}
