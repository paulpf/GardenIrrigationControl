#ifndef GARDENCONTROLLERAPP_H
#define GARDENCONTROLLERAPP_H

#include <array>

#include "connectivitycoordinator.h"
#include "esp32waterlevelsensor.h"
#include "irrigation_zone.h"
#include "loopscheduler.h"
#include "mqttmanager.h"
#include "otamanager.h"
#include "waterlevelmanager.h"
#include "wifimanager.h"

class GardenControllerApp
{
public:
  GardenControllerApp();

  void setup();
  void loop();

private:
  struct ZoneConfig
  {
    int buttonPin;
    int relayPin;
  };

  static const int CLIENT_NAME_MAX_SIZE = 50;
  static const std::array<ZoneConfig, MAX_IRRIGATION_ZONES> ZONE_CONFIGS;

  char _clientName[CLIENT_NAME_MAX_SIZE];

  WifiManager _wifiManager;
  MqttManager _mqttManager;
  OtaManager _otaManager;
  IrrigationZone _irrigationZones[MAX_IRRIGATION_ZONES];
  Esp32WaterLevelSensor _waterLevelSensor;
  WaterLevelManager _waterLevelManager;
  ConnectivityCoordinator _connectivityCoordinator;
  LoopScheduler _loopScheduler;

  unsigned long _currentMillis = 0;
  unsigned long _previousMillisLongLoop = 0;
  unsigned long _previousMillisMiddleLoop = 0;
  unsigned long _previousMillisShortLoop = 0;
  unsigned long _mainLoopStartTime = 0;

  void initIrrigationZones();
  void updateClientNameFromMac();
  void setupOta();
  void initWatchdog();
  void waitForWifiConnection();
  void plotZoneStates(unsigned long currentTime);
  void handleShortIntervalTasks();
  bool handleOtaUpdate();
  void updateLoopTimingPlot();
  void handleMiddleIntervalEvents();
  void handleLongIntervalTasks();
};

#endif // GARDENCONTROLLERAPP_H
