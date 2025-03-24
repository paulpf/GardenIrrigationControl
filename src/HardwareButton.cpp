#include "HardwareButton.h"

HardwareButton::HardwareButton(int gpioChannel, unsigned long debounceDelay, void (*callback)())
    : gpioChannel(gpioChannel), debounceDelay(debounceDelay), lastDebounceTime(0), callback(callback) {}

void HardwareButton::setup() {
    pinMode(gpioChannel, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(gpioChannel), callback, RISING);
}

void IRAM_ATTR HardwareButton::onPressed() {
    unsigned long now = millis();
    if (now - lastDebounceTime > debounceDelay) {
        lastDebounceTime = now;
        callback();
    }
}