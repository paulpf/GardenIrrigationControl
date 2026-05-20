#include <Arduino.h>

#include "gpio_manager.h"
#include "ota_update.h"
#include "wifi_manager.h"

namespace {
unsigned long lastToggle = 0;
bool ledState = false;
constexpr unsigned long LED_TOGGLE_INTERVAL_MS = 500;
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[System] Starting ESP32 template");

  GPIOManager::pinMode(GPIOManager::PinFunction::LED, OUTPUT);
  GPIOManager::pinMode(GPIOManager::PinFunction::BUTTON1, INPUT_PULLUP);
  GPIOManager::pinMode(GPIOManager::PinFunction::RELAY1, OUTPUT);
  GPIOManager::digitalWrite(GPIOManager::PinFunction::RELAY1, HIGH);

  WiFi_Manager::begin();
  OTA_Update::begin();
}

void loop() {
  WiFi_Manager::loop();
  OTA_Update::handle();

  const unsigned long now = millis();
  if (now - lastToggle >= LED_TOGGLE_INTERVAL_MS) {
    lastToggle = now;
    ledState = !ledState;
    GPIOManager::digitalWrite(GPIOManager::PinFunction::LED,
                              ledState ? HIGH : LOW);
  }
}
