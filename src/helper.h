// helper.h

#ifndef HELPER_H
#define HELPER_H

#include "global_defines.h"
#include "irrigation_zone.h"
#include "mqttmanager.h"

class Helper
{
public:
  static String replaceChars(const String &str, char charsToReplace,
                             char replaceWith);

  // New memory-efficient methods
  static char *createCharArray(const String &str);
  static void concatToBuffer(char *buffer, size_t bufferSize, const char *str1,
                             const char *str2);
  static void formatToBuffer(char *buffer, size_t bufferSize,
                             const char *format, ...);
  static void addIrrigationZone(int buttonPin, int relayPin,
                                IrrigationZone *irrigationZones,
                                MqttManager *mqttManager, int index,
                                const char *clientNameBuffer);

  // Uptime formatting
  static String formatUptime(unsigned long uptimeMillis);

  // Memory formatting
  static String formatMemory(unsigned long bytes);
};

#endif