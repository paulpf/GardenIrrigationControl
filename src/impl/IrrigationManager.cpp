// IrrigationManager.cpp
#include "../_interfaces/IrrigationManager.h"

IrrigationManager::IrrigationManager()
{
  
}

IrrigationManager::~IrrigationManager()
{
  for (int i = 0; i < 8; i++)
  {
    delete irrigationZones[i];
  }
}

void IrrigationManager::setup()
{
  // Create irrigation zones
  irrigationZones[0] = new IrrigationZone("Zone 1", 33, "Addr1");
}

void IrrigationManager::readInputs()
{
  // Read inputs from all irrigation zones
  for (int i = 0; i < 8; i++)
  {
    //irrigationZones[i]->readInputs();
  }
}

void IrrigationManager::processLogic()
{
  // Process logic for all irrigation zones
  for (int i = 0; i < 8; i++)
  {
    //irrigationZones[i]->processLogic();
  }
}

void IrrigationManager::writeOutputs()
{
  // Write outputs for all irrigation zones
  for (int i = 0; i < 8; i++)
  {
    //irrigationZones[i]->writeOutputs();
  }
}
