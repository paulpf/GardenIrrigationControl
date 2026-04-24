#ifndef IRRIGATIONZONEFACTORY_H
#define IRRIGATIONZONEFACTORY_H

#include "hardwareconfig.h"
#include "irrigationconfig.h"
#include "irrigation_zone.h"
#include "mqttmanager.h"

class IrrigationZoneFactory
{
public:
  static void initializeZone(IrrigationZone &zone,
                             const IrrigationConfig &irrigationConfig,
                             const HardwareConfig::ZonePins &zonePins,
                             MqttManager &mqttManager, int zoneIndex,
                             const char *clientNameBuffer);
};

#endif // IRRIGATIONZONEFACTORY_H