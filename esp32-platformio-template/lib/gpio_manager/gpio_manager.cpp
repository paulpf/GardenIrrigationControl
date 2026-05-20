#include <limits.h>

#include "gpio_manager.h"

namespace {
constexpr uint8_t INVALID_PIN = UCHAR_MAX;
}  // namespace

const GPIOManager::PinMapEntry
    GPIOManager::pinMap[static_cast<uint8_t>(PinFunction::COUNT)] = {
        {PinFunction::OUTPUT_0, 2}, // GPIO 2 = onboard LED on most ESP32 DevKit boards
        {PinFunction::INPUT_0,  0}, // replace with your actual input pin
};

uint8_t GPIOManager::resolvePin(PinFunction pin) {
  const uint8_t index = static_cast<uint8_t>(pin);
  if (index >= static_cast<uint8_t>(PinFunction::COUNT)) {
    return INVALID_PIN;
  }
  return pinMap[index].gpio;
}

void GPIOManager::pinMode(PinFunction pin, uint8_t mode) {
  const uint8_t gpio = resolvePin(pin);
  if (gpio == INVALID_PIN) {
    return;
  }
  ::pinMode(gpio, mode);
}

void GPIOManager::digitalWrite(PinFunction pin, uint8_t value) {
  const uint8_t gpio = resolvePin(pin);
  if (gpio == INVALID_PIN) {
    return;
  }
  ::digitalWrite(gpio, value);
}

int GPIOManager::digitalRead(PinFunction pin) {
  const uint8_t gpio = resolvePin(pin);
  if (gpio == INVALID_PIN) {
    return LOW;
  }
  return ::digitalRead(gpio);
}
