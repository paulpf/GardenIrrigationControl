// ChannelController.h
#ifndef CHANNELCONTROLLER_H
#define CHANNELCONTROLLER_H

#pragma once


#include <Arduino.h>

class ChannelController
{
public:
  ChannelController(int channel_pin);
  ~ChannelController();
  void setup();
  void setChannel(bool state);
  int getChannel();

private:
  int channel_pin;
};

#endif // CHANNELCONTROLLER_H