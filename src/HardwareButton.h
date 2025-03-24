#ifndef HARDWARE_BUTTON_H
#define HARDWARE_BUTTON_H

#include <Arduino.h>

class HardwareButton {
public:
    HardwareButton(int gpioChannel, unsigned long debounceDelay, void (*callback)());
    void setup();
    void IRAM_ATTR onPressed();

private:
    int gpioChannel;
    unsigned long debounceDelay;
    unsigned long lastDebounceTime;
    void (*callback)();
};

#endif // HARDWARE_BUTTON_H