#ifndef GARDENCONTROLLERBOOTSTRAP_H
#define GARDENCONTROLLERBOOTSTRAP_H

#include "arduinotimeprovider.h"
#include "connectivitycoordinator.h"
#include "esp32waterlevelsensor.h"
#include "gardencontrollerapp.h"
#include "hardwareconfig.h"
#include "irrigation_zone.h"
#include "irrigationconfig.h"
#include "loopscheduler.h"
#include "mqttmanager.h"
#include "otaloopguard.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "waterlevelmanager.h"
#include "wificonnectionawaiter.h"
#include "wifimanager.h"

class GardenControllerBootstrap
{
public:
  GardenControllerBootstrap();

  GardenControllerApp &application();

private:
  IrrigationConfig _irrigationConfig;
  SystemConfig _systemConfig;
  HardwareConfig _hardwareConfig;

  WifiManager _wifiManager;
  MqttManager _mqttManager;
  OtaManager _otaManager;
  IrrigationZone _irrigationZones[MAX_IRRIGATION_ZONES];
  Esp32WaterLevelSensor _waterLevelSensor;
  WaterLevelManager _waterLevelManager;
  ConnectivityCoordinator _connectivityCoordinator;
  ArduinoTimeProvider _timeProvider;
  WifiConnectionAwaiter _wifiConnectionAwaiter;
  OtaLoopGuard _otaLoopGuard;
  LoopScheduler _loopScheduler;
  GardenControllerApp _app;
};

#endif // GARDENCONTROLLERBOOTSTRAP_H
