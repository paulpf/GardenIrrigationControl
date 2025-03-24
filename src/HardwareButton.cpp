#include "HardwareButton.h"
#include <Arduino.h>
#include <functional>

HardwareButton* HardwareButton::instance = nullptr;

HardwareButton::HardwareButton(int gpioChannel, unsigned long debounceDelay, void (*callback)())
    : gpioChannel(gpioChannel), debounceDelay(debounceDelay), lastDebounceTime(0), callback(callback) {}

void HardwareButton::setup() {
    pinMode(gpioChannel, INPUT_PULLDOWN);
    instance = this;
    attachInterrupt(digitalPinToInterrupt(gpioChannel), HardwareButton::onPressedStatic, RISING);
}

void IRAM_ATTR HardwareButton::onPressedStatic() {
    if (instance) {
        instance->onPressed();
    }
}

void IRAM_ATTR HardwareButton::onPressed() {
    unsigned long now = millis();
    if (now - lastDebounceTime > debounceDelay) {
        lastDebounceTime = now;
        callback();
    }
}