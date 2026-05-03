#pragma once

#include "ArduinoFake.h"

#include <map>

#define IRAM_ATTR

constexpr int INPUT = 0;
constexpr int OUTPUT = 1;
constexpr int INPUT_PULLDOWN = 2;
constexpr int LOW = 0;
constexpr int HIGH = 1;
constexpr int RISING = 1;
constexpr int ADC_11db = 0;

inline unsigned long &arduinoFakeMillisStorage()
{
  static unsigned long value = 0;
  return value;
}

inline std::map<int, int> &arduinoFakeDigitalPinStateStorage()
{
  static std::map<int, int> pinStates;
  return pinStates;
}

inline void arduinoFakeSetMillis(unsigned long value)
{
  arduinoFakeMillisStorage() = value;
}

inline void arduinoFakeSetDigitalPinState(int pin, int value)
{
  arduinoFakeDigitalPinStateStorage()[pin] = value;
}

inline void arduinoFakeReset()
{
  arduinoFakeMillisStorage() = 0;
  arduinoFakeDigitalPinStateStorage().clear();
}

inline void pinMode(int, int)
{
}
inline void digitalWrite(int, int)
{
}
inline int digitalPinToInterrupt(int pin)
{
  return pin;
}
inline int digitalRead(int pin)
{
  auto &pinStates = arduinoFakeDigitalPinStateStorage();
  auto it = pinStates.find(pin);
  return it == pinStates.end() ? LOW : it->second;
}
inline void attachInterruptArg(int, void (*)(void *), void *, int)
{
}
inline unsigned long millis()
{
  return arduinoFakeMillisStorage();
}
inline void delay(unsigned long)
{
}
inline void yield()
{
}
inline void analogSetPinAttenuation(int, int)
{
}
inline int analogRead(int)
{
  return 0;
}

class HardwareSerial
{
public:
  void begin(unsigned long)
  {
  }
  void println(const char *)
  {
  }
  void print(const char *)
  {
  }
};

static HardwareSerial Serial;
