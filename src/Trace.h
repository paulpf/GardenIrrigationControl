#ifndef TRACE_H
#define TRACE_H

#include "globaldefines.h"
#include "config.h"

/**
 * Represents different trace levels for logging
 */
enum class TraceLevel 
{
  DEBUG,  // Debug information for development
  INFO,   // Regular informational messages (lowest priority)  
  ERROR   // Error messages (highest priority)
};

class Trace
{
public:
  static void log(TraceLevel level, String message);
  
  // Helper methods for specific levels
  static void info(String message) { log(TraceLevel::INFO, message); }
  static void debug(String message) { log(TraceLevel::DEBUG, message); }
  static void error(String message) { log(TraceLevel::ERROR, message); }
  
private:
  // Check if the message should be logged based on the configured level
  static bool shouldLog(TraceLevel level) {
    return static_cast<int>(level) >= static_cast<int>(TRACE_LEVEL);
  }
};

#endif // TRACE_H