#ifndef AUTOMATICIRRIGATIONCOMMANDHANDLER_H
#define AUTOMATICIRRIGATIONCOMMANDHANDLER_H

#include <Arduino.h>

#include "automaticirrigationcontroller.h"
#include "automaticirrigationpublisher.h"

class AutomaticIrrigationCommandHandler
{
public:
  AutomaticIrrigationCommandHandler(
      AutomaticIrrigationController &controller,
      AutomaticIrrigationPublisher &publisher);

  void setup(const char *clientName);
  bool handleCommand(const char *topic, const char *payload);
  const char *getEnabledSetTopic() const;

private:
  AutomaticIrrigationController &_controller;
  AutomaticIrrigationPublisher &_publisher;
  String _enabledSetTopic;

  bool parseBoolPayload(const char *payload, bool &value) const;
};

#endif // AUTOMATICIRRIGATIONCOMMANDHANDLER_H
