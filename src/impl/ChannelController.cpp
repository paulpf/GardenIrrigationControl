// ChannelController.cpp
#include "../_interfaces/ChannelController.h"

ChannelController::ChannelController(int channel_pin)
{
  this->channel_pin = channel_pin;
}

ChannelController::~ChannelController()
{
}

void ChannelController::setup()
{
  pinMode(channel_pin, OUTPUT);
}

void ChannelController::setChannel(bool state)
{
  digitalWrite(channel_pin, state);
}

int ChannelController::getChannel()
{
  return digitalRead(channel_pin);
}



