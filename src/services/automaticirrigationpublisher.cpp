#include "automaticirrigationpublisher.h"

AutomaticIrrigationPublisher::AutomaticIrrigationPublisher(
    IMessagePublisher &publisher, AutomaticIrrigationController &controller)
    : _publisher(publisher), _controller(controller)
{
}

void AutomaticIrrigationPublisher::setup(const char *clientName)
{
  _enabledTopic = String(clientName) + "/irrigation/automatic/enabled";
}

void AutomaticIrrigationPublisher::publishState()
{
  if (!_publisher.isConnected() || _enabledTopic.length() == 0)
  {
    return;
  }

  _publisher.publish(_enabledTopic.c_str(),
                     _controller.isEnabled() ? "true" : "false");
}
