#include "helper.h"

String Helper::replaceChars(const String &str, char findChar, char replaceChar) 
{
    String result = str;
    result.replace(findChar, replaceChar);
    return result;
}

// Improved string handling to avoid heap fragmentation
char* Helper::createCharArray(const String &str) 
{
    size_t len = str.length() + 1;
    char* buffer = new char[len];
    str.toCharArray(buffer, len);
    return buffer;
}

// Memory-efficient concat operation for strings
void Helper::concatToBuffer(char* buffer, size_t bufferSize, const char* str1, const char* str2) 
{
    strncpy(buffer, str1, bufferSize);
    size_t len1 = strlen(str1);
    if (len1 < bufferSize - 1) 
    {
        strncat(buffer, str2, bufferSize - len1 - 1);
    }
    buffer[bufferSize - 1] = '\0'; // Ensuring null termination
}

// Formatted output to a buffer
void Helper::formatToBuffer(char* buffer, size_t bufferSize, const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, bufferSize, format, args);
    va_end(args);
}

// Example for using addIrrigationZone
bool Helper::addIrrigationZone(int buttonPin, int relayPin, IrrigationZone* irrigationZones, MqttManager* mqttManager, int &activeZones, const char* clientNameBuffer) 
{
    if (activeZones < MAX_IRRIGATION_ZONES) 
    {
      char topicBuffer[100];
      snprintf(topicBuffer, sizeof(topicBuffer), "%s/irrigationZone%d", clientNameBuffer, activeZones + 1);
      
      irrigationZones[activeZones].setup(buttonPin, relayPin, topicBuffer);
      mqttManager->addIrrigationZone(&irrigationZones[activeZones]);
      activeZones++;
      Trace::log(TraceLevel::DEBUG, "New irrigation zone added: " + String(topicBuffer) + " (Zone " + String(activeZones) + ")");
      return true;
    } 
    else 
    {
      Trace::log(TraceLevel::DEBUG, "Maximum number of irrigation zones reached!");
      return false;
    }
}