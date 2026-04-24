#ifndef GARDENCONTROLLERAPP_H
#define GARDENCONTROLLERAPP_H

#include <array>

#include "arduinotimeprovider.h"
#include "connectivitycoordinator.h"
#include "esp32waterlevelsensor.h"
#include "hardwareconfig.h"
#include "irrigation_zone.h"
#include "irrigationconfig.h"
#include "irrigationzonefactory.h"
#include "loopscheduler.h"
#include "mqttmanager.h"
#include "otaloopguard.h"
#include "otamanager.h"
#include "systemconfig.h"
#include "waterlevelmanager.h"
#include "wificonnectionawaiter.h"
#include "wifimanager.h"

class GardenControllerApp
{
public:
  GardenControllerApp(IrrigationConfig &irrigationConfig,
                      SystemConfig &systemConfig,
                      HardwareConfig &hardwareConfig, WifiManager &wifiManager,
                      MqttManager &mqttManager, OtaManager &otaManager,
                      IrrigationZone (&irrigationZones)[MAX_IRRIGATION_ZONES],
                      WaterLevelManager &waterLevelManager,
                      ConnectivityCoordinator &connectivityCoordinator,
                      WifiConnectionAwaiter &wifiConnectionAwaiter,
                      OtaLoopGuard &otaLoopGuard, LoopScheduler &loopScheduler);

  void setup();
  void loop();

private:
  static const int CLIENT_NAME_MAX_SIZE = 50;

  char _clientName[CLIENT_NAME_MAX_SIZE];
  IrrigationConfig &_irrigationConfig;
  SystemConfig &_systemConfig;
  HardwareConfig &_hardwareConfig;

  WifiManager &_wifiManager;
  MqttManager &_mqttManager;
  OtaManager &_otaManager;
  IrrigationZone (&_irrigationZones)[MAX_IRRIGATION_ZONES];
  WaterLevelManager &_waterLevelManager;
  ConnectivityCoordinator &_connectivityCoordinator;
  WifiConnectionAwaiter &_wifiConnectionAwaiter;
  OtaLoopGuard &_otaLoopGuard;
  LoopScheduler &_loopScheduler;

  unsigned long _currentMillis = 0;
  unsigned long _previousMillisLongLoop = 0;
  unsigned long _previousMillisMiddleLoop = 0;
  unsigned long _previousMillisShortLoop = 0;
  unsigned long _mainLoopStartTime = 0;

  void initializeStorage();
  void initializeIdentity();
  void initializeConnectivity();
  void initializeSubsystems();
  void initializeRuntimeSafety();

  void initIrrigationZones();
  void updateClientNameFromMac();
  void setupOta();
  void initWatchdog();
  void waitForWifiConnection();
  void plotZoneStates(unsigned long currentTime);
  void handleShortIntervalTasks();
  void updateLoopTimingPlot();
  void handleMiddleIntervalEvents();
  void handleLongIntervalTasks();
};

#endif // GARDENCONTROLLERAPP_H
