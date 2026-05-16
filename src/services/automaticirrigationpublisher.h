#ifndef AUTOMATICIRRIGATIONPUBLISHER_H
#define AUTOMATICIRRIGATIONPUBLISHER_H

#include <Arduino.h>

#include "automaticirrigationcontroller.h"
#include "imessagepublisher.h"

class AutomaticIrrigationPublisher
{
public:
  AutomaticIrrigationPublisher(IMessagePublisher &publisher,
                               AutomaticIrrigationController &controller);

  void setup(const char *clientName);
  void publishState();

private:
  IMessagePublisher &_publisher;
  AutomaticIrrigationController &_controller;
  String _enabledTopic;
};

#endif // AUTOMATICIRRIGATIONPUBLISHER_H
