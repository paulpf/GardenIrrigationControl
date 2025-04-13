// tools.h

#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>
#include <stdarg.h>

class Tools
{
public:
  static String replaceChars(const String &str, char charsToReplace, char replaceWith);
  
  // Neue speichereffiziente Methoden
  static char* createCharArray(const String &str);
  static void concatToBuffer(char* buffer, size_t bufferSize, const char* str1, const char* str2);
  static void formatToBuffer(char* buffer, size_t bufferSize, const char* format, ...);
  static bool addIrrigationZone(int buttonPin, int relayPin, const char* zoneName, IrrigationZone* irrigationZones, MqttManager* mqttManager, int &activeZones, const char* clientNameBuffer);
};

#endif