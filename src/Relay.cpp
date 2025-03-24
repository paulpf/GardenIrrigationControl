#include "Relay.h"
#include <Arduino.h>
#include "Trace.h"

Relay::Relay(int gpioChannel) : gpioChannel(gpioChannel), state(false) {
}

void Relay::setup() {
    pinMode(gpioChannel, OUTPUT);
    digitalWrite(gpioChannel, HIGH); // Set relay to OFF initially
}

void Relay::switchRelay(bool newState) {
    state = newState;
    if (state) {
        Trace::log("Switching relay ON");
        digitalWrite(gpioChannel, LOW); // Active low
    } else {
        Trace::log("Switching relay OFF");
        digitalWrite(gpioChannel, HIGH); // Active low
    }
}

bool Relay::getState() const {
    return digitalRead(gpioChannel) == LOW;
}