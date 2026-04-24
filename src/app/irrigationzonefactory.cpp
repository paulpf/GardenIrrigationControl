#include "irrigationzonefactory.h"

#include "trace.h"

void IrrigationZoneFactory::initializeZone(
    IrrigationZone &zone, const IrrigationConfig &irrigationConfig,
    const HardwareConfig::ZonePins &zonePins, MqttManager &mqttManager,
    int zoneIndex, const char *clientNameBuffer)
{
  char topicBuffer[100];
  snprintf(topicBuffer, sizeof(topicBuffer), "%s/irrigationZone%d",
           clientNameBuffer, zoneIndex + 1);

  Trace::log(TraceLevel::INFO,
             "Initializing irrigation zone: " + String(topicBuffer) +
                 " (Zone " + String(zoneIndex) + ")");

  zone.configure(irrigationConfig);
  zone.setup(zonePins.buttonPin, zonePins.relayPin, topicBuffer);
  mqttManager.addIrrigationZone(&zone);
}