// IrrigationZone.cpp

#include "../_interfaces/IrrigationZone.h"

IrrigationZone::IrrigationZone(String name, int hwButtonGpioPin, String swButtonAddress, int relaisGpioPin) : 
    _name(name),
    _hwButton(hwButtonGpioPin),
    _swButtonAddress(swButtonAddress),
    _relais(relaisGpioPin)
{

}

IrrigationZone::~IrrigationZone()
{
    // Nothing to do here
}

void IrrigationZone::readInputs()
{
    // Read inputs from hardware button
    _hwButtonIsPressed = _hwButton.isPressed();
}

void IrrigationZone::processLogic()
{
    
}

void IrrigationZone::writeOutputs()
{
    // Write outputs to relais
    _relais.setRelaisState(_hwButtonIsPressed ? OPEN : CLOSE);
}