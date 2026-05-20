#pragma once

#include <Arduino.h>

class GPIOManager {
 public:
  enum class PinFunction : uint8_t {
    OUTPUT_0 = 0, // e.g. onboard LED or any digital output
    INPUT_0  = 1, // e.g. a push button or any digital input
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
