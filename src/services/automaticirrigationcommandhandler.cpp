#include "automaticirrigationcommandhandler.h"

#include "storage_manager.h"
#include "trace.h"
#include <cstring>

AutomaticIrrigationCommandHandler::AutomaticIrrigationCommandHandler(
    AutomaticIrrigationController &controller,
    AutomaticIrrigationPublisher &publisher)
    : _controller(controller), _publisher(publisher)
{
}

void AutomaticIrrigationCommandHandler::setup(const char *clientName)
{
  _enabledSetTopic = String(clientName) + "/irrigation/automatic/enabled/set";
}

bool AutomaticIrrigationCommandHandler::handleCommand(const char *topic,
                                                      const char *payload)
{
  if (topic == nullptr || payload == nullptr || _enabledSetTopic.length() == 0)
  {
    return false;
  }

  if (strcmp(topic, _enabledSetTopic.c_str()) != 0)
  {
    return false;
  }

  bool enabled = false;
  if (!parseBoolPayload(payload, enabled))
  {
    Trace::log(TraceLevel::ERROR,
               "Invalid payload for automatic irrigation enabled: " +
                   String(payload));
    return true;
  }

  _controller.setEnabled(enabled);
  StorageManager::getInstance().saveAutomaticIrrigationEnabled(enabled);
  _publisher.publishState();
  Trace::log(TraceLevel::INFO,
             "Updated automatic irrigation enabled to " +
                 String(enabled ? "true" : "false"));
  return true;
}

const char *AutomaticIrrigationCommandHandler::getEnabledSetTopic() const
{
  return _enabledSetTopic.c_str();
}

bool AutomaticIrrigationCommandHandler::parseBoolPayload(const char *payload,
                                                         bool &value) const
{
  if (strcmp(payload, "true") == 0 || strcmp(payload, "1") == 0 ||
      strcmp(payload, "on") == 0)
  {
    value = true;
    return true;
  }

  if (strcmp(payload, "false") == 0 || strcmp(payload, "0") == 0 ||
      strcmp(payload, "off") == 0)
  {
    value = false;
    return true;
  }

  return false;
}
