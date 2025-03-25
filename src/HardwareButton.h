#ifndef HARDWARE_BUTTON_H
#define HARDWARE_BUTTON_H

#include "GlobalDefines.h"

class HardwareButton {
public:
    static HardwareButton* instance;
    HardwareButton(int gpioChannel, unsigned long debounceDelay, void (*callback)());
    void setup();
    void IRAM_ATTR onPressed();
    static void IRAM_ATTR onPressedStatic();

private:
    int gpioChannel;
    unsigned long debounceDelay;
    unsigned long lastDebounceTime;
    void (*callback)();
};

#endif // HARDWARE_BUTTON_H