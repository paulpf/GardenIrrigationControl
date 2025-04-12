// tools.h

#ifndef TOOLS_H
#define TOOLS_H

#include <Arduino.h>

class Tools
{
public:
  static String replaceChars(String str, char charsToReplace, char replaceWith);
};

#endif