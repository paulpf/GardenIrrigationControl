#pragma once

#include <Arduino.h>

class GPIOManager {
 public:
  enum class PinFunction : uint8_t {
    LED = 0,
    RELAY1 = 1,
    BUTTON1 = 2,
    COUNT
  };

  static void pinMode(PinFunction pin, uint8_t mode);
  static void digitalWrite(PinFunction pin, uint8_t value);
  static int digitalRead(PinFunction pin);

 private:
  struct PinMapEntry {
    PinFunction function;
    uint8_t gpio;
  };

  static uint8_t resolvePin(PinFunction pin);
  static const PinMapEntry pinMap[static_cast<uint8_t>(PinFunction::COUNT)];
};
