#include <Arduino.h>

#include "gpio_manager.h"
#include "ota_update.h"
#include "wifi_manager.h"

namespace {
unsigned long lastToggle = 0;
bool outputState = false;
constexpr unsigned long TOGGLE_INTERVAL_MS = 500;
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  GPIOManager::pinMode(GPIOManager::PinFunction::OUTPUT_0, OUTPUT);
  GPIOManager::pinMode(GPIOManager::PinFunction::INPUT_0, INPUT_PULLUP);

  WiFi_Manager::begin();
  OTA_Update::begin();
}

void loop() {
  WiFi_Manager::loop();
  OTA_Update::handle();

  const unsigned long now = millis();
  if (now - lastToggle >= TOGGLE_INTERVAL_MS) {
    lastToggle = now;
    outputState = !outputState;
    GPIOManager::digitalWrite(GPIOManager::PinFunction::OUTPUT_0,
                              outputState ? HIGH : LOW);
  }
}
