#pragma once

class Preferences
{
public:
  bool begin(const char *, bool = false)
  {
    return true;
  }
  void end()
  {
  }
  void clear()
  {
  }
  void putInt(const char *, int)
  {
  }
  int getInt(const char *, int defaultValue = 0)
  {
    return defaultValue;
  }
  void putBool(const char *, bool)
  {
  }
  bool getBool(const char *, bool defaultValue = false)
  {
    return defaultValue;
  }
};
