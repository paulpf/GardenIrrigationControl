// IrrigationZone.cpp

#include "../_interfaces/IrrigationZone.h"

IrrigationZone::IrrigationZone(String name, int gpioChannel)
{
  this->name = name;
  this->valve = new Valve(name, gpioChannel);
}

IrrigationZone::~IrrigationZone()
{
  delete valve;
}

int IrrigationZone::getDuration()
{
  return duration;
}

void IrrigationZone::setDuration(int duration)
{
  this->duration = duration;
}

void IrrigationZone::start()
{
    unsigned long currentMillis = millis();

    // Only start a new irrigation if we're not already running
    if (valve->getValveState() == CLOSE)
    {
        valve->setValveState(OPEN);
        timeLeft = duration;
        previousMillis = currentMillis;
        isActive = true;
    }

    // Update timer if irrigation is active
    if (isActive)
    {
        // Check if one second has passed (1000 milliseconds)
        if (currentMillis - previousMillis >= 1000)
        {
            previousMillis = currentMillis;
            timeLeft--;

            // Check if irrigation duration has completed
            if (timeLeft <= 0)
            {
                valve->setValveState(CLOSE);
                isActive = false;
                timeLeft = 0;
            }
        }
    }
}

void IrrigationZone::stop()
{
    valve->setValveState(CLOSE);
    isActive = false;
    timeLeft = 0;
}