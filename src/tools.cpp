#include "tools.h"
#include "globaldefines.h"
#include "config.h"
#include "irrigationZone.h"
#include "mqttmanager.h"

String Tools::replaceChars(const String &str, char findChar, char replaceChar) 
{
    String result = str;
    result.replace(findChar, replaceChar);
    return result;
}

// Verbesserte String-Handhabung zur Vermeidung von Heap-Fragmentierung
char* Tools::createCharArray(const String &str) 
{
    size_t len = str.length() + 1;
    char* buffer = new char[len];
    str.toCharArray(buffer, len);
    return buffer;
}

// Speichereffiziente concat-Operation für Strings
void Tools::concatToBuffer(char* buffer, size_t bufferSize, const char* str1, const char* str2) 
{
    strncpy(buffer, str1, bufferSize);
    size_t len1 = strlen(str1);
    if (len1 < bufferSize - 1) 
    {
        strncat(buffer, str2, bufferSize - len1 - 1);
    }
    buffer[bufferSize - 1] = '\0'; // Sicherstellung der Nullterminierung
}

// Formatierte Ausgabe in einen Puffer
void Tools::formatToBuffer(char* buffer, size_t bufferSize, const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, bufferSize, format, args);
    va_end(args);
}

// Beispiel für die Verwendung von addIrrigationZone
bool Tools::addIrrigationZone(int buttonPin, int relayPin, const char* zoneName, IrrigationZone* irrigationZones, MqttManager* mqttManager, int &activeZones, const char* clientNameBuffer) 
{
    if (activeZones < MAX_IRRIGATION_ZONES) 
    {
      char topicBuffer[100];
      snprintf(topicBuffer, sizeof(topicBuffer), "%s/irrigationZone%d", clientNameBuffer, activeZones + 1);
      
      irrigationZones[activeZones].setup(buttonPin, relayPin, topicBuffer);
      mqttManager->addIrrigationZone(&irrigationZones[activeZones]);
      activeZones++;
      Trace::log("Neue Bewässerungszone hinzugefügt: " + String(zoneName) + " (Zone " + String(activeZones) + ")");
      return true;
    } 
    else 
    {
      Trace::log("Maximale Anzahl an Bewässerungszonen erreicht!");
      return false;
    }
}