#include "HardwareButton.h"

HardwareButton* HardwareButton::instance = nullptr;

HardwareButton::HardwareButton(int gpioChannel, unsigned long debounceDelay)
    : gpioChannel(gpioChannel), debounceDelay(debounceDelay), lastDebounceTime(0) {}

void HardwareButton::setOnPressedCallback(const std::function<void()>& callback) {
    this->onPressedCallback = callback;
}

void HardwareButton::setup() {
    // Setup code for the hardware button
    pinMode(gpioChannel, INPUT_PULLDOWN);
    instance = this;
    attachInterrupt(digitalPinToInterrupt(gpioChannel), []() { instance->onPressed(); }, RISING);
}

void IRAM_ATTR HardwareButton::onPressed() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > debounceDelay) {
        lastDebounceTime = currentTime;
        Trace::log("Hardware button pressed");
        
        if (onPressedCallback) {
            onPressedCallback(); // Call the stored callback
        }
    }
}

// void IRAM_ATTR HardwareButton::onPressedStatic() {
//     if (instance) {
//         instance->onPressed();
//     }
// }