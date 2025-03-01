// Relais.cpp
#include "../_interfaces/Relais.h"

Relais::Relais(int gpioChannel) : 
    _gpioChannel(gpioChannel)
{
    pinMode(_gpioChannel, OUTPUT);
    digitalWrite(_gpioChannel, LOW);
}

Relais::~Relais()
{
    // Nothing to do here
}

RelaisState Relais::getRelaisState()
{
    return _state;
}

void Relais::setRelaisState(RelaisState state)
{
    _state = state;
    digitalWrite(_gpioChannel, state == OPEN ? HIGH : LOW);
}