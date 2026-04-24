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

// Format uptime from milliseconds to "Xd Yh Zm" format
String Helper::formatUptime(unsigned long uptimeMillis) 
{
    unsigned long uptimeSeconds = uptimeMillis / 1000;
    
    unsigned long days = uptimeSeconds / 86400;  // 86400 seconds in a day
    uptimeSeconds %= 86400;
    
    unsigned long hours = uptimeSeconds / 3600;  // 3600 seconds in an hour
    uptimeSeconds %= 3600;
    
    unsigned long minutes = uptimeSeconds / 60;
    
    String result = "";
    if (days > 0) {
        result += String(days) + " d ";
    }
    if (hours > 0 || days > 0) {
        result += String(hours) + " h ";
    }
    result += String(minutes) + " m"; // Always show minutes, even if less than an hour
    
    return result;
}

// Format memory from bytes to KB format (without "KB" string)
String Helper::formatMemory(unsigned long bytes) 
{
    float kb = bytes / 1024.0;
    return String(kb, 1);
}