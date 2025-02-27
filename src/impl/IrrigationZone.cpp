// IrrigationZone.cpp

#include "../_interfaces/IrrigationZone.h"

IrrigationZone::IrrigationZone(String name, int hwButtonGpioPin, String swButtonAddress)
    : _name(name), _hwButton(hwButtonGpioPin), _swButtonAddress(swButtonAddress)
{

}

IrrigationZone::~IrrigationZone()
{
    // Nothing to do here
}