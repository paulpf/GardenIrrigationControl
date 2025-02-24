// IrrigationManager.cpp
#include "../_interfaces/IrrigationManager.h"

IrrigationManager::IrrigationManager()
{
  
}

IrrigationManager::~IrrigationManager()
{
  delete drainageValve;

  for (int i = 0; i < 8; i++)
  {
    delete irrigationZones[i];
  }
}

void IrrigationManager::setup()
{
  // Create drainage valve
  drainageValve = new Valve("Drainage Valve", 32);

  // Create irrigation zones
  irrigationZones[0] = new IrrigationZone("Zone 1", 33);
  irrigationZones[1] = new IrrigationZone("Zone 2", 34);  
  irrigationZones[2] = new IrrigationZone("Zone 3", 35);
  irrigationZones[3] = new IrrigationZone("Zone 4", 36);
  irrigationZones[4] = new IrrigationZone("Zone 5", 37);
  irrigationZones[5] = new IrrigationZone("Zone 6", 38);
  irrigationZones[6] = new IrrigationZone("Zone 7", 39);
  irrigationZones[7] = new IrrigationZone("Zone 8", 40);  

  // Set initial drainage valve state
  drainageValve->setValveState(CLOSE);

  // Set initial irrigation zone states
  for (int i = 0; i < 8; i++)
  {
    irrigationZones[i]->setDuration(0);
  }
}

void IrrigationManager::updateZone(int zoneIndex)
{
  irrigationZones[zoneIndex]->start();
}

void IrrigationManager::stopZone(int zoneIndex)
{
  irrigationZones[zoneIndex]->stop();
}