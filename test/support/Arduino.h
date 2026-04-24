#pragma once

#include "ArduinoFake.h"

#define IRAM_ATTR

constexpr int INPUT = 0;
constexpr int OUTPUT = 1;
constexpr int INPUT_PULLDOWN = 2;
constexpr int LOW = 0;
constexpr int HIGH = 1;
constexpr int RISING = 1;
constexpr int ADC_11db = 0;

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline int digitalPinToInterrupt(int pin) { return pin; }
inline void attachInterruptArg(int, void (*)(void *), void *, int) {}
inline unsigned long millis() { return 0; }
inline void delay(unsigned long) {}
inline void yield() {}
inline void analogSetPinAttenuation(int, int) {}
inline int analogRead(int) { return 0; }

class HardwareSerial
{
public:
  void begin(unsigned long) {}
  void println(const char *) {}
  void print(const char *) {}
};

static HardwareSerial Serial;
