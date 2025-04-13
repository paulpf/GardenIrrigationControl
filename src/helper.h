// helper.h

#ifndef HELPER_H
#define HELPER_H

#include "globaldefines.h"
#include "irrigationZone.h"
#include "mqttmanager.h"

class Helper
{
public:
  static String replaceChars(const String &str, char charsToReplace, char replaceWith);
  
  // New memory-efficient methods
  static char* createCharArray(const String &str);
  static void concatToBuffer(char* buffer, size_t bufferSize, const char* str1, const char* str2);
  static void formatToBuffer(char* buffer, size_t bufferSize, const char* format, ...);
  static bool addIrrigationZone(int buttonPin, int relayPin, IrrigationZone* irrigationZones, MqttManager* mqttManager, int &activeZones, const char* clientNameBuffer);
};

#endif